#include "intrinsics.h"

#include <llvm/IR/IRBuilder.h>

#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"


void createSystemCall(std::unique_ptr<Context> &context, std::string functionName,
                      std::vector<FunctionArgument> functionparams, std::shared_ptr<VariableType> returnType)
{
    if (llvm::Function *F = context->FunctionDefinitions[functionName]; F == nullptr)
    {
        std::vector<llvm::Type *> params;
        for (const auto &param: functionparams)
        {
            params.push_back(param.type->generateLlvmType(context));
        }
        llvm::Type *resultType = llvm::Type::getVoidTy(*context->TheContext);
        if (returnType)
        {
            resultType = returnType->generateLlvmType(context);
        }

        llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, false);

        F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, functionName, context->TheModule.get());

        // Set names for all arguments.
        unsigned idx = 0;
        for (auto &arg: F->args())
            arg.setName(functionparams[idx++].argumentName);
    }
}

void createPrintfCall(const std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;
    params.push_back(llvm::PointerType::getUnqual(*context->TheContext));

    llvm::Type *resultType = llvm::Type::getInt32Ty(*context->TheContext);
    llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, true);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "printf", context->TheModule.get());
    for (auto &arg: F->args())
        arg.setName("__fmt");
}


void writeLnCodegen(std::unique_ptr<Context> &context, const size_t length)
{
    std::vector<llvm::Type *> params;
    std::string m_name = "writeln(integer" + std::to_string(length) + ")";
    auto type = VariableType::getInteger(length);
    params.push_back(type->generateLlvmType(context));

    llvm::Type *resultType = llvm::Type::getVoidTy(*context->TheContext);
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
    else if (length == 8)
        ArgsV.push_back(context->Builder->CreateGlobalString("%c\n"));
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
    std::string m_name = "writeln(string)";
    auto type = VariableType::getString();
    params.push_back(llvm::PointerType::getUnqual(*context->TheContext));

    llvm::Type *resultType = llvm::Type::getVoidTy(*context->TheContext);
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

    const auto stringStructPtr = F->getArg(0);
    const auto arrayPointerOffset =
            context->Builder->CreateStructGEP(type->generateLlvmType(context), stringStructPtr, 2, "string.ptr.offset");
    const auto value =
            context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), arrayPointerOffset);
    std::vector<llvm::Value *> ArgsV;
    ArgsV.push_back(context->Builder->CreateGlobalString("%s\n"));

    ArgsV.push_back(value);

    context->Builder->CreateCall(CalleeF, ArgsV);

    context->Builder->CreateRetVoid();

    verifyFunction(*F, &llvm::errs());
    context->FunctionDefinitions[m_name] = F;
}
