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
