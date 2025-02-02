#include "codegen.h"

#include <llvm/IR/IRBuilder.h>

#include "compiler/Context.h"

llvm::Value *codegen::codegen_ifexpr(std::unique_ptr<Context> &context, llvm::Value *condition,
                                     std::function<void(std::unique_ptr<Context> &)> body)
{
    llvm::Value *CondV = condition;
    if (!CondV)
        return nullptr;
    CondV = context->Builder->CreateICmpEQ(CondV, context->Builder->getTrue(), "ifcond");

    llvm::Function *TheFunction = context->Builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*context->TheContext, "then", TheFunction);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*context->TheContext, "ifcont");

    context->Builder->CreateCondBr(CondV, ThenBB, MergeBB);

    context->Builder->SetInsertPoint(ThenBB);

    body(context);

    if (!context->BreakBlock.BlockUsed)
        context->Builder->CreateBr(MergeBB);
    context->BreakBlock.BlockUsed = false;
    TheFunction->insert(TheFunction->end(), MergeBB);
    context->Builder->SetInsertPoint(MergeBB);

    return CondV;
}
