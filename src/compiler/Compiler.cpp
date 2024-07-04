#include "compiler/Compiler.h"
#include "Lexer.h"
#include "Parser.h"
#include "ast/FunctionDefinitionNode.h"
#include "compiler/Context.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"

#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/TargetParser/Host.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/// printd - printf that takes a double prints it as "%f\n", returning 0.
extern "C" DLLEXPORT void writeln(double X)
{
    fprintf(stdout, "%f\n", X);
}

std::unique_ptr<Context> InitializeModule(const char *module_name)
{
    auto context = std::make_unique<Context>();
    // Open a new context and module.
    context->TheContext = std::make_unique<llvm::LLVMContext>();
    context->TheModule = std::make_unique<llvm::Module>(module_name, *context->TheContext);

    // Create a new builder for the module.
    context->Builder = std::make_unique<llvm::IRBuilder<>>(*context->TheContext);
    return std::move<>(context);
}

void createSystemCall(std::unique_ptr<Context> &context, std::string functionName, std::vector<FunctionArgument> functionparams)
{
    llvm::Function *F = context->FunctionDefinitions[functionName];
    if (F == nullptr)
    {
        std::vector<llvm::Type *> params;
        for (auto &param : functionparams)
        {
            params.push_back(param.type.generateLlvmType(context));
        }
        llvm::Type *resultType = llvm::Type::getVoidTy(*context->TheContext);

        llvm::FunctionType *FT =
            llvm::FunctionType::get(resultType, params, false);

        llvm::Function *F =
            llvm::Function::Create(FT, llvm::Function::ExternalLinkage, functionName, context->TheModule.get());

        // Set names for all arguments.
        unsigned idx = 0;
        for (auto &arg : F->args())
            arg.setName(functionparams[idx++].argumentName);
    }
}

void createPrintfCall(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;
    params.push_back(llvm::Type::getInt8PtrTy(*context->TheContext));
    llvm::Type *resultType = llvm::Type::getInt32Ty(*context->TheContext);
    llvm::FunctionType *FT =
        llvm::FunctionType::get(resultType, params, true);
    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "printf", context->TheModule.get());
    for (auto &arg : F->args())
        arg.setName("__fmt");
}

void createSystemCall(std::unique_ptr<Context> &context, std::string functionName, std::vector<FunctionArgument> functionparams, VariableType returnType)
{

    llvm::Function *F = context->FunctionDefinitions[functionName];
    if (F == nullptr)
    {
        std::vector<llvm::Type *> params;
        for (auto &param : functionparams)
        {
            params.push_back(param.type.generateLlvmType(context));
        }
        llvm::Type *resultType = returnType.generateLlvmType(context);

        llvm::FunctionType *FT =
            llvm::FunctionType::get(resultType, params, false);

        llvm::Function *F =
            llvm::Function::Create(FT, llvm::Function::ExternalLinkage, functionName, context->TheModule.get());

        // Set names for all arguments.
        unsigned idx = 0;
        for (auto &arg : F->args())
            arg.setName(functionparams[idx++].argumentName);
    }
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
    auto context = InitializeModule(unit->getUnitName().c_str());
    VariableType intType{.baseType = VariableBaseType::Integer, .typeName = "integer"};
    createPrintfCall(context);
    createSystemCall(context, "exit", {FunctionArgument{.type = intType, .argumentName = "X", .isReference = false}});

    unit->codegen(context);
    std::cerr << "start printing code\n";
    context->TheModule->print(llvm::errs(), nullptr, false, true);

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

    auto TheTargetMachine = Target->createTargetMachine(
        TargetTriple, CPU, Features, opt, Reloc::PIC_);

    context->TheModule->setDataLayout(TheTargetMachine->createDataLayout());

    auto Filename = unit->getUnitName() + ".o";
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
}