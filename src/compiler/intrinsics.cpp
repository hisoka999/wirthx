#include "intrinsics.h"

#include <llvm/IR/IRBuilder.h>

#include "ast/types/FileType.h"
#include "ast/types/StringType.h"
#include "codegen.h"
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
void createAssignCall(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;
    auto fileType = FileType::getFileType();
    auto llvmFileType = fileType->generateLlvmType(context);
    params.push_back(llvm::PointerType::getUnqual(*context->TheContext));
    params.push_back(llvm::PointerType::getUnqual(*context->TheContext));

    llvm::Type *resultType = llvm::Type::getVoidTy(*context->TheContext);
    llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::PrivateLinkage, "assignfile(file,string)",
                                               context->TheModule.get());
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context->TheContext, "_block", F);
    context->Builder->SetInsertPoint(BB);

    llvm::Function *CalleeF = context->TheModule->getFunction("fopen");
    std::vector<llvm::Value *> ArgsV;
    //
    auto stringStructPtr = F->getArg(1);
    auto type = StringType::getString()->generateLlvmType(context);
    const auto arrayPointerOffset = context->Builder->CreateStructGEP(type, stringStructPtr, 2, "string.ptr.offset");
    auto stringPathPointer =
            context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), arrayPointerOffset);
    //
    ArgsV.push_back(stringPathPointer);
    ArgsV.push_back(context->Builder->CreateGlobalStringPtr("rw"));
    const auto callResult = context->Builder->CreateCall(CalleeF, ArgsV);
    const auto resultPointer = context->Builder->CreatePointerCast(callResult, context->Builder->getInt64Ty());

    //  codegen::codegen_ifexpr(context, condition,
    //                          [](std::unique_ptr<Context> ctx)
    //                          {
    //                              // TODO
    //                          });

    llvm::Value *condition =
            context->Builder->CreateCmp(llvm::CmpInst::ICMP_EQ, resultPointer, context->Builder->getInt64(0));
    codegen::codegen_ifexpr(context, condition,
                            [](std::unique_ptr<Context> &ctx)
                            {
                                llvm::Function *callExit = ctx->TheModule->getFunction("exit");
                                std::vector<llvm::Value *> ArgsV = {ctx->Builder->getInt32(1)};

                                ctx->Builder->CreateCall(callExit, ArgsV);
                            });
    auto fileName = context->Builder->CreateStructGEP(llvmFileType, F->getArg(0), 0, "file.name");
    auto filePtr = context->Builder->CreateStructGEP(llvmFileType, F->getArg(0), 1, "file.ptr");
    context->Builder->CreateStore(callResult, filePtr);
    context->Builder->CreateStore(arrayPointerOffset, fileName);

    context->Builder->CreateRetVoid();
}
void createReadLnCall(std::unique_ptr<Context> &context)
{


    std::vector<llvm::Type *> params;
    auto fileType = FileType::getFileType();
    auto llvmFileType = fileType->generateLlvmType(context);
    auto llvmStringType = StringType::getString()->generateLlvmType(context);
    params.push_back(llvm::PointerType::getUnqual(*context->TheContext));
    params.push_back(llvm::PointerType::getUnqual(*context->TheContext));

    llvm::Type *resultType = llvm::Type::getVoidTy(*context->TheContext);
    llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, false);
    llvm::Function *F =
            llvm::Function::Create(FT, llvm::Function::PrivateLinkage, "readln(file,string)", context->TheModule.get());
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context->TheContext, "_block", F);
    context->Builder->SetInsertPoint(BB);
    F->getArg(0)->setName("file");
    F->getArg(1)->setName("value");
    //


    auto filePtrOffset = context->Builder->CreateStructGEP(llvmFileType, F->getArg(0), 1, "file.ptr");
    auto filePtr = context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), filePtrOffset);

    auto stringPtr = context->Builder->CreateStructGEP(llvmStringType, F->getArg(1), 2, "string.ptr");
    auto stringSizeOffset = context->Builder->CreateStructGEP(llvmStringType, F->getArg(1), 1, "string.size");


    const auto resultPointer = context->Builder->CreatePointerCast(filePtr, context->Builder->getInt64Ty());

    llvm::Value *condition =
            context->Builder->CreateCmp(llvm::CmpInst::ICMP_NE, resultPointer, context->Builder->getInt64(0));
    codegen::codegen_ifexpr(
            context, condition,
            [filePtr, stringPtr, stringSizeOffset](std::unique_ptr<Context> &ctx)
            {
                auto sizePtr =
                        ctx->Builder->CreateAlloca(llvm::Type::getInt64Ty(*ctx->TheContext), nullptr, "size.ptr");
                auto valuePtrPtr = ctx->Builder->CreateAlloca(llvm::PointerType::getUnqual(*ctx->TheContext));
                ctx->Builder->CreateStore(ctx->Builder->getInt64(0), valuePtrPtr);
                llvm::Function *CalleeF = ctx->TheModule->getFunction("getline");
                std::vector<llvm::Value *> ArgsV = {valuePtrPtr, sizePtr, filePtr};
                auto result = ctx->Builder->CreateCall(CalleeF, ArgsV);
                // memcopy value to string
                auto memcpyCall = llvm::Intrinsic::getDeclaration(
                        ctx->TheModule.get(), llvm::Intrinsic::memcpy,
                        {ctx->Builder->getPtrTy(), ctx->Builder->getPtrTy(), ctx->Builder->getInt64Ty()});
                auto size = ctx->Builder->CreateAdd(result, ctx->Builder->getInt32(-1));
                //
                // auto size64 = ctx->Builder->CreateIntCast(size, llvm::Type::getInt64Ty(*ctx->TheContext), true);
                ctx->Builder->CreateStore(size, stringSizeOffset);

                std::vector<llvm::Value *> memcopyArgs;
                const auto valueType = VariableType::getInteger(8)->generateLlvmType(ctx);

                const auto boundsLhs = ctx->Builder->CreateGEP(
                        valueType, stringPtr, llvm::ArrayRef<llvm::Value *>{ctx->Builder->getInt64(0)}, "", false);

                // const auto loadResult =
                //         ctx->Builder->CreateLoad(llvm::PointerType::getUnqual(*ctx->TheContext), valuePtrPtr);
                memcopyArgs.push_back(boundsLhs);
                memcopyArgs.push_back(valuePtrPtr);
                memcopyArgs.push_back(size);
                memcopyArgs.push_back(ctx->Builder->getFalse());

                ctx->Builder->CreateCall(memcpyCall, memcopyArgs);

                const auto bounds = ctx->Builder->CreateGEP(
                        valueType, stringPtr,
                        llvm::ArrayRef<llvm::Value *>{ctx->Builder->CreateAdd(size, ctx->Builder->getInt32(1))}, "",
                        false);

                ctx->Builder->CreateStore(ctx->Builder->getInt8(0), bounds);
            });


    // context->Builder->CreateStore(, filePtr);

    context->Builder->CreateRetVoid();
}
void createCloseFileCall(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;

    auto fileType = FileType::getFileType();
    auto llvmFileType = fileType->generateLlvmType(context);
    params.push_back(llvm::PointerType::getUnqual(*context->TheContext));

    llvm::Type *resultType = llvm::Type::getVoidTy(*context->TheContext);
    llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, false);
    llvm::Function *F =
            llvm::Function::Create(FT, llvm::Function::PrivateLinkage, "closefile(file)", context->TheModule.get());
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context->TheContext, "_block", F);
    context->Builder->SetInsertPoint(BB);
    //


    auto filePtrOffset = context->Builder->CreateStructGEP(llvmFileType, F->getArg(0), 1, "file.ptr");
    auto filePtr = context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), filePtrOffset);
    const auto resultPointer = context->Builder->CreatePointerCast(filePtr, context->Builder->getInt64Ty());

    llvm::Value *condition =
            context->Builder->CreateCmp(llvm::CmpInst::ICMP_NE, resultPointer, context->Builder->getInt64(0));
    codegen::codegen_ifexpr(context, condition,
                            [filePtr](std::unique_ptr<Context> &ctx)
                            {
                                llvm::Function *CalleeF = ctx->TheModule->getFunction("fclose");
                                std::vector<llvm::Value *> ArgsV = {filePtr};
                                ctx->Builder->CreateCall(CalleeF, ArgsV);
                            });


    // context->Builder->CreateStore(, filePtr);

    context->Builder->CreateRetVoid();
}
