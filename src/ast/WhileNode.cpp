#include "WhileNode.h"

#include <llvm/IR/IRBuilder.h>

#include <utility>

#include "compiler/Context.h"
#include "exceptions/CompilerException.h"

WhileNode::WhileNode(const Token &token, std::shared_ptr<ASTNode> loopCondition,
                     std::vector<std::shared_ptr<ASTNode>> nodes) :
    ASTNode(token), m_loopCondition(std::move(loopCondition)), m_nodes(std::move(nodes))
{
}

void WhileNode::print() {}

llvm::Value *WhileNode::codegen(std::unique_ptr<Context> &context)
{
    llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(*context->TheContext, "loop", context->TopLevelFunction);
    llvm::BasicBlock *LoopCondBB =
            llvm::BasicBlock::Create(*context->TheContext, "loop.cond", context->TopLevelFunction);

    llvm::BasicBlock *AfterBB = llvm::BasicBlock::Create(*context->TheContext, "afterloop", context->TopLevelFunction);

    // Insert an explicit fall through from the current block to the LoopBB.
    context->Builder->CreateBr(LoopCondBB);
    context->Builder->SetInsertPoint(LoopCondBB);
    // // Convert condition to a bool by comparing non-equal to 0.0.
    // EndCond = context->Builder->CreateICmpEQ(
    //     EndCond, llvm::ConstantInt::get(*context->TheContext, llvm::APInt(64, 0)), "loopcond");
    // Create the "after loop" block and insert it.
    // Compute the end condition.
    llvm::Value *EndCond = m_loopCondition->codegen(context);
    if (!EndCond)
        return nullptr;


    // Insert the conditional branch into the end of LoopEndBB.
    context->Builder->CreateCondBr(EndCond, LoopBB, AfterBB);


    // Start insertion in LoopBB.
    context->Builder->SetInsertPoint(LoopBB);

    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB.  Note that we ignore the value computed by the body, but don't
    // allow an error.
    for (auto &node: m_nodes)
    {
        context->Builder->SetInsertPoint(LoopBB);
        node->codegen(context);
    }
    context->Builder->CreateBr(LoopCondBB);

    // Any new code will be inserted in AfterBB.
    context->Builder->SetInsertPoint(AfterBB);

    // for expr always returns 0.0.
    return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(*context->TheContext));
}
void WhileNode::typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    if (const auto &conditionType = m_loopCondition->resolveType(unit, parentNode))
    {
        if (conditionType->baseType != VariableBaseType::Boolean)
        {
            throw CompilerException(ParserError{.token = expressionToken(),
                                                .message = "the loop expression does not return a boolean"});
        }
    }


    for (const auto &node: m_nodes)
    {
        node->typeCheck(unit, parentNode);
    }
}
