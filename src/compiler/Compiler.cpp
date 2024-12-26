#include "compiler/Compiler.h"

#include <filesystem>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "ast/FunctionDefinitionNode.h"

#include "compiler/Context.h"
#include "compiler/intrinsics.h"
#include "linker/pascal_linker.h"
#include "llvm/IR/PassManager.h"

#include "llvm/IR/LegacyPassManager.h"

#include "llvm/IR/Verifier.h"
#include "llvm/LTO/LTO.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include "os/command.h"

std::unique_ptr<Context> InitializeModule(std::unique_ptr<UnitNode> &unit, const CompilerOptions &options)
{
    auto context = std::make_unique<Context>();
    context->compilerOptions = options;
    // Open a new context and module.
    context->TheContext = std::make_unique<llvm::LLVMContext>();
    context->TheModule = std::make_unique<llvm::Module>(unit->getUnitName(), *context->TheContext);

    // Create a new builder for the module.
    context->Builder = std::make_unique<llvm::IRBuilder<>>(*context->TheContext);
    // Create new pass and analysis managers.
    context->TheFPM = std::make_unique<llvm::FunctionPassManager>();
    const auto TheLAM = std::make_unique<llvm::LoopAnalysisManager>();
    context->TheFAM = std::make_unique<llvm::FunctionAnalysisManager>();
    const auto TheCGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
    context->TheMAM = std::make_unique<llvm::ModuleAnalysisManager>();
    context->ThePIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
    context->TheSI = std::make_unique<llvm::StandardInstrumentations>(*context->TheContext,
                                                                      /*DebugLogging*/ true);

    context->TheSI->registerCallbacks(*context->ThePIC, context->TheMAM.get());


    // Add transform passes.
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    context->TheFPM->addPass(llvm::InstCombinePass());
    // Reassociate expressions.
    context->TheFPM->addPass(llvm::ReassociatePass());
    // Eliminate Common SubExpressions.
    context->TheFPM->addPass(llvm::GVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    context->TheFPM->addPass(llvm::SimplifyCFGPass());


    // context->TheFPM->addPass(llvm::createLoopSimplifyPass());

    // Register analysis passes used in these transform passes.
    llvm::PassBuilder PB;

    PB.registerModuleAnalyses(*context->TheMAM);
    PB.registerFunctionAnalyses(*context->TheFAM);
    PB.crossRegisterProxies(*TheLAM, *context->TheFAM, *TheCGAM, *context->TheMAM);

    context->ProgramUnit = std::move(unit);
    return context;
}


void compile_file(const CompilerOptions &options, std::filesystem::path inputPath, std::ostream &errorStream,
                  std::ostream &outputStream)
{
    std::ifstream file;
    std::istringstream is;

    file.open(inputPath, std::ios::in);
    if (!file.is_open())
    {
        return;
    }
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);

    Lexer lexer;

    auto tokens = lexer.tokenize(inputPath.string(), buffer);

    Parser parser(options.rtlDirectories, inputPath, tokens);
    auto unit = parser.parseUnit();
    if (parser.hasError())
    {
        parser.printErrors(errorStream);
        return;
    }
    auto context = InitializeModule(unit, options);
    auto intType = VariableType::getInteger();

    createPrintfCall(context);
    createSystemCall(context, "exit", {FunctionArgument{.type = intType, .argumentName = "X", .isReference = false}});
    createSystemCall(
            context, "malloc",
            {FunctionArgument{.type = VariableType::getInteger(64), .argumentName = "size", .isReference = false}},
            VariableType::getPointer());

    using namespace llvm;
    using namespace llvm::sys;
    // Initialize the target registry etc.InitializeAllTargetInfos();
    // InitializeAllTargets();
    // InitializeAllTargetMCs();
    // InitializeAllAsmParsers();
    // InitializeAllAsmPrinters();

    InitializeNativeTarget();
    InitializeNativeTargetAsmParser();
    InitializeNativeTargetAsmPrinter();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    context->TheModule->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target)
    {
        errs() << Error;
        return;
    }

    auto CPU = "generic";
    auto Features = "";

    TargetOptions opt;
    auto TheTargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, opt, Reloc::PIC_);

    context->TheModule->setDataLayout(TheTargetMachine->createDataLayout());


    try
    {
        context->ProgramUnit->typeCheck(context->ProgramUnit, nullptr);
        context->ProgramUnit->codegen(context);
    }
    catch (CompilerException &e)
    {
        errorStream << e.what();
        return;
    }


    auto basePath = context->compilerOptions.outputDirectory;


    auto objectFileName = basePath / (context->ProgramUnit->getUnitName() + ".o");
    std::vector<std::string> objectFiles;
    objectFiles.emplace_back(objectFileName.string());
    std::error_code EC;
    raw_fd_ostream dest(objectFileName.string(), EC, sys::fs::OF_None);

    if (EC)
    {
        errs() << "Could not open file: " << EC.message();
        return;
    }

    legacy::PassManager pass;
    if (context->compilerOptions.buildMode == BuildMode::Release)
    {
        pass.add(llvm::createAlwaysInlinerLegacyPass());
        pass.add(llvm::createInstructionCombiningPass());
    }

    auto FileType = CodeGenFileType::ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    {
        errs() << "TheTargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*context->TheModule);
    dest.flush();


    llvm::verifyModule(*context->TheModule, &llvm::errs());
    if (context->compilerOptions.printLLVMIR)
    {
        context->TheModule->print(llvm::errs(), nullptr, false, false);
    }

    outs() << "Wrote " << objectFileName << "\n";

    std::vector<std::string> flags;
    for (const auto &lib: context->ProgramUnit->collectLibsToLink())
    {
        flags.push_back("-l" + lib);
    }

    if (!pascal_link_modules(errorStream, basePath, context->ProgramUnit->getUnitName(), flags, objectFiles))
    {
        return;
    }

    if (context->compilerOptions.runProgram)
    {

        if (!execute_command(outputStream, (basePath / context->ProgramUnit->getUnitName()).string()))
        {
            errorStream << "program could not be executed!\n";
        }
    }
}
