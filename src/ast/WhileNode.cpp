#include "WhileNode.h"
#include <interpreter/InterpreterContext.h>
#include "compiler/Context.h"

WhileNode::WhileNode(std::shared_ptr<ASTNode> loopCondition, std::vector<std::shared_ptr<ASTNode>> nodes) :
    m_loopCondition(loopCondition), m_nodes(nodes)
{
}

void WhileNode::print() {}

void WhileNode::eval(InterpreterContext &context, std::ostream &outputStream)
{

    while (true)
    {
        m_loopCondition->eval(context, outputStream);
        auto value = context.stack.pop_front<int64_t>();
        if (value == 0)
            break;

        for (auto &node: m_nodes)
        {
            node->eval(context, outputStream);
            if (context.stack.stopBreakIfActive())
                return;
        }
    }
}

llvm::Value *WhileNode::codegen(std::unique_ptr<Context> &context)
{
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(*context->TheContext, "loop", context->TopLevelFunction);

    // Insert an explicit fall through from the current block to the LoopBB.
    context->Builder->CreateBr(LoopBB);

    // Start insertion in LoopBB.
    context->Builder->SetInsertPoint(LoopBB);

    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB.  Note that we ignore the value computed by the body, but don't
    // allow an error.
    for (auto &node: m_nodes)
    {
        node->codegen(context);
    }

    // // Convert condition to a bool by comparing non-equal to 0.0.
    // EndCond = context->Builder->CreateICmpEQ(
    //     EndCond, llvm::ConstantInt::get(*context->TheContext, llvm::APInt(64, 0)), "loopcond");
    // Create the "after loop" block and insert it.
    // Compute the end condition.
    llvm::Value *EndCond = m_loopCondition->codegen(context);
    if (!EndCond)
        return nullptr;

    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(*context->TheContext, "afterloop", context->TopLevelFunction);

    // Insert the conditional branch into the end of LoopEndBB.
    context->Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

    // Any new code will be inserted in AfterBB.
    context->Builder->SetInsertPoint(AfterBB);

    // for expr always returns 0.0.
    return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(*context->TheContext));
}
