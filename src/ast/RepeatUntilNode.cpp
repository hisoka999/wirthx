#include "RepeatUntilNode.h"

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>

#include "compiler/Context.h"

RepeatUntilNode::RepeatUntilNode(std::shared_ptr<ASTNode> loopCondition, std::vector<std::shared_ptr<ASTNode>> nodes) :
    m_loopCondition(std::move(loopCondition)), m_nodes(std::move(nodes))
{
}
void RepeatUntilNode::print() {}
llvm::Value *RepeatUntilNode::codegen(std::unique_ptr<Context> &context)
{

    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(*context->TheContext, "repeat", context->TopLevelFunction);
    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(*context->TheContext, "afteruntil", context->TopLevelFunction);

    // Insert an explicit fall through from the current block to the LoopBB.
    context->Builder->CreateBr(LoopBB);
    context->Builder->SetInsertPoint(LoopBB);


    // Start insertion in LoopBB.
    context->Builder->SetInsertPoint(LoopBB);
    context->BreakBlock.Block = AfterBB;
    context->BreakBlock.BlockUsed = false;
    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB.  Note that we ignore the value computed by the body, but don't
    // allow an error.
    for (auto &node: m_nodes)
    {
        context->Builder->SetInsertPoint(LoopBB);
        node->codegen(context);
    }

    context->BreakBlock.Block = nullptr;

    // // Convert condition to a bool by comparing non-equal to 0.0.

    // Create the "after loop" block and insert it.
    // Compute the end condition.
    llvm::Value *EndCond = m_loopCondition->codegen(context);
    if (!EndCond)
        return nullptr;

    // Insert the conditional branch into the end of LoopEndBB.
    context->Builder->CreateCondBr(EndCond, AfterBB, LoopBB);


    // Any new code will be inserted in AfterBB.
    context->Builder->SetInsertPoint(AfterBB);

    // for expr always returns 0.0.
    return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(*context->TheContext));
}