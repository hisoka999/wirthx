#include "compiler/Compiler.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "ast/FunctionDefinitionNode.h"
#include "compiler/Context.h"
#include "linker/pascal_linker.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

std::unique_ptr<Context> InitializeModule(std::unique_ptr<UnitNode> &unit)
{
    auto context = std::make_unique<Context>();
    // Open a new context and module.
    context->TheContext = std::make_unique<llvm::LLVMContext>();
    context->TheModule = std::make_unique<llvm::Module>(unit->getUnitName(), *context->TheContext);

    // Create a new builder for the module.
    context->Builder = std::make_unique<llvm::IRBuilder<>>(*context->TheContext);
    // Create new pass and analysis managers.
    context->TheFPM = std::make_unique<llvm::FunctionPassManager>();
    context->TheLAM = std::make_unique<llvm::LoopAnalysisManager>();
    context->TheFAM = std::make_unique<llvm::FunctionAnalysisManager>();
    context->TheCGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
    context->TheMAM = std::make_unique<llvm::ModuleAnalysisManager>();
    context->ThePIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
    context->TheSI = std::make_unique<llvm::StandardInstrumentations>(*context->TheContext,
                                                                      /*DebugLogging*/ true);

    context->TheSI->registerCallbacks(*context->ThePIC, context->TheFAM.get());

    // Add transform passes.
    // Do simple "peephole" optimizations and bit-twiddling optzns.
    context->TheFPM->addPass(llvm::InstCombinePass());
    // Reassociate expressions.
    context->TheFPM->addPass(llvm::ReassociatePass());
    // Eliminate Common SubExpressions.
    context->TheFPM->addPass(llvm::GVNPass());
    // Simplify the control flow graph (deleting unreachable blocks, etc).
    context->TheFPM->addPass(llvm::SimplifyCFGPass());


    // Register analysis passes used in these transform passes.
    llvm::PassBuilder PB;
    PB.registerModuleAnalyses(*context->TheMAM);
    PB.registerFunctionAnalyses(*context->TheFAM);
    PB.crossRegisterProxies(*context->TheLAM, *context->TheFAM, *context->TheCGAM, *context->TheMAM);

    context->ProgramUnit = std::move(unit);
    return context;
}

void createSystemCall(std::unique_ptr<Context> &context, std::string functionName,
                      std::vector<FunctionArgument> functionparams)
{
    llvm::Function *F = context->FunctionDefinitions[functionName];
    if (F == nullptr)
    {
        std::vector<llvm::Type *> params;
        for (auto &param: functionparams)
        {
            params.push_back(param.type->generateLlvmType(context));
        }
        llvm::Type *resultType = llvm::Type::getVoidTy(*context->TheContext);

        llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, false);

        llvm::Function *F =
                llvm::Function::Create(FT, llvm::Function::ExternalLinkage, functionName, context->TheModule.get());

        // Set names for all arguments.
        unsigned idx = 0;
        for (auto &arg: F->args())
            arg.setName(functionparams[idx++].argumentName);
    }
}

void createPrintfCall(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;
    params.push_back(llvm::Type::getInt8PtrTy(*context->TheContext));
    llvm::Type *resultType = llvm::Type::getInt32Ty(*context->TheContext);
    llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, true);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "printf", context->TheModule.get());
    for (auto &arg: F->args())
        arg.setName("__fmt");
}

void createSystemCall(std::unique_ptr<Context> &context, std::string functionName,
                      std::vector<FunctionArgument> functionparams, VariableType returnType)
{

    llvm::Function *F = context->FunctionDefinitions[functionName];
    if (F == nullptr)
    {
        std::vector<llvm::Type *> params;
        for (auto &param: functionparams)
        {
            params.push_back(param.type->generateLlvmType(context));
        }
        llvm::Type *resultType = returnType.generateLlvmType(context);

        llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, false);

        llvm::Function *F =
                llvm::Function::Create(FT, llvm::Function::ExternalLinkage, functionName, context->TheModule.get());

        // Set names for all arguments.
        unsigned idx = 0;
        for (auto &arg: F->args())
            arg.setName(functionparams[idx++].argumentName);
    }
}

void writeLnCodegen(std::unique_ptr<Context> &context, size_t length)
{
    std::vector<llvm::Type *> params;
    std::string m_name = "writeln_int" + std::to_string(length);
    llvm::Type *resultType;
    auto type = VariableType::getInteger(length);
    params.push_back(type->generateLlvmType(context));

    resultType = llvm::Type::getVoidTy(*context->TheContext);
    llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, false);

    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, m_name, context->TheModule.get());
    F->addFnAttr(llvm::Attribute::AlwaysInline);
    // Set names for all arguments.
    for (auto &arg: F->args())
        arg.setName("arg");

    // Create a new basic block to start insertion into.

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context->TheContext, m_name, F);

    context->Builder->SetInsertPoint(BB);

    llvm::Function *CalleeF = context->TheModule->getFunction("printf");
    if (!CalleeF)
        LogErrorV("Unknown function referenced");

    std::vector<llvm::Value *> ArgsV;
    if (length > 32)
        ArgsV.push_back(context->Builder->CreateGlobalString("%ld\n"));
    else
        ArgsV.push_back(context->Builder->CreateGlobalString("%d\n"));
    for (auto &arg: F->args())
        ArgsV.push_back(F->getArg(arg.getArgNo()));

    context->Builder->CreateCall(CalleeF, ArgsV);

    context->Builder->CreateRetVoid();

    verifyFunction(*F, &llvm::errs());
    context->FunctionDefinitions[m_name] = F;
}

void writeLnStrCodegen(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;
    std::string m_name = "writeln_str";
    llvm::Type *resultType;
    auto type = VariableType::getString();
    params.push_back(type->generateLlvmType(context));

    resultType = llvm::Type::getVoidTy(*context->TheContext);
    llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, true);

    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, m_name, context->TheModule.get());
    F->addFnAttr(llvm::Attribute::AlwaysInline);
    // Set names for all arguments.
    for (auto &arg: F->args())
        arg.setName("arg");

    // Create a new basic block to start insertion into.

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context->TheContext, m_name, F);

    context->Builder->SetInsertPoint(BB);

    llvm::Function *CalleeF = context->TheModule->getFunction("printf");
    if (!CalleeF)
        LogErrorV("Unknown function referenced");

    std::vector<llvm::Value *> ArgsV;
    ArgsV.push_back(context->Builder->CreateGlobalString("%s\n"));
    for (auto &arg: F->args())
        ArgsV.push_back(F->getArg(arg.getArgNo()));

    context->Builder->CreateCall(CalleeF, ArgsV);

    context->Builder->CreateRetVoid();

    verifyFunction(*F, &llvm::errs());
    context->FunctionDefinitions[m_name] = F;
}
void compile_file(std::filesystem::path inputPath, std::ostream &errorStream, std::ostream &outputStream)
{
    std::ifstream file;
    std::istringstream is;

    file.open(inputPath, std::ios::in);
    if (!file.is_open())
    {
        return;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);

    Lexer lexer;

    auto tokens = lexer.tokenize(std::string_view{buffer});

    Parser parser(inputPath, tokens);
    auto unit = parser.parseUnit();
    if (parser.hasError())
    {
        parser.printErrors(errorStream);
        return;
    }
    auto context = InitializeModule(unit);
    auto intType = VariableType::getInteger();
    llvm::Intrinsic::getDeclaration(context->TheModule.get(), llvm::Intrinsic::vastart);
    llvm::Intrinsic::getDeclaration(context->TheModule.get(), llvm::Intrinsic::vacopy);

    llvm::Intrinsic::getDeclaration(context->TheModule.get(), llvm::Intrinsic::vaend);

    createPrintfCall(context);
    writeLnCodegen(context, 32);
    writeLnCodegen(context, 64);
    writeLnStrCodegen(context);
    createSystemCall(context, "exit", {FunctionArgument{.type = intType, .argumentName = "X", .isReference = false}});
    try
    {
        context->ProgramUnit->codegen(context);
    }
    catch (CompilerException &e)
    {
        errorStream << e.what();
        return;
    }


    llvm::verifyModule(*context->TheModule, &llvm::errs());

    context->TheModule->print(llvm::errs(), nullptr, false, false);

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

    auto Filename = context->ProgramUnit->getUnitName() + ".o";
    std::vector<std::string> objectFiles;
    objectFiles.emplace_back(Filename);
    std::error_code EC;
    raw_fd_ostream dest(Filename, EC, sys::fs::OF_None);

    if (EC)
    {
        errs() << "Could not open file: " << EC.message();
        return;
    }

    legacy::PassManager pass;
    auto FileType = CodeGenFileType::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
    {
        errs() << "TheTargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*context->TheModule);
    dest.flush();

    outs() << "Wrote " << Filename << "\n";
    // TODO link object files
    auto basePath = std::filesystem::current_path();
    pascal_link_modules(basePath, context->ProgramUnit->getUnitName(), {"-lc"}, objectFiles);
}
