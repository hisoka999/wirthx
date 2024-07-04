#include "IfConditionNode.h"
#include "compiler/Context.h"
#include "interpreter/Stack.h"
#include <iostream>

IfConditionNode::IfConditionNode(std::shared_ptr<ASTNode> conditionNode, std::vector<std::shared_ptr<ASTNode>> ifExpressions, std::vector<std::shared_ptr<ASTNode>> elseExpressions)
    : m_conditionNode(conditionNode), m_ifExpressions(ifExpressions), m_elseExpressions(elseExpressions)
{
}

void IfConditionNode::print()
{
    std::cout << "if ";
    m_conditionNode->print();
    std::cout << " then\n";

    for (auto &exp : m_ifExpressions)
    {
        exp->print();
    }
    if (m_elseExpressions.size() > 0)
    {
        std::cout << "else\n";
        for (auto &exp : m_elseExpressions)
        {
            exp->print();
        }
        // std::cout << "end;\n";
    }
}

void IfConditionNode::eval(Stack &stack, std::ostream &outputStream)
{
    m_conditionNode->eval(stack, outputStream);
    // check result
    auto result = stack.pop_front<int64_t>();
    if (result)
    {
        for (auto &exp : m_ifExpressions)
        {
            exp->eval(stack, outputStream);
        }
    }
    else
    {
        for (auto &exp : m_elseExpressions)
        {
            exp->eval(stack, outputStream);
        }
    }
}

llvm::Value *IfConditionNode::codegen(std::unique_ptr<Context> &context)
{

    llvm::Value *CondV = m_conditionNode->codegen(context);
    if (!CondV)
        return nullptr;

    // Convert condition to a bool by comparing non-equal to 0.0.
    CondV = context->Builder->CreateFCmpONE(
        CondV, llvm::ConstantFP::get(*context->TheContext, llvm::APFloat(0.0)), "ifcond");
    llvm::Function *TheFunction = context->Builder->GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    llvm::BasicBlock *ThenBB =
        llvm::BasicBlock::Create(*context->TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*context->TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*context->TheContext, "ifcont");

    // Emit then value.
    context->Builder->SetInsertPoint(ThenBB);

    for (auto &exp : m_ifExpressions)
    {
        exp->codegen(context);
    }

    context->Builder->CreateBr(MergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    ThenBB = context->Builder->GetInsertBlock();

    context->Builder->CreateCondBr(CondV, ThenBB, ElseBB);
    // Emit merge block.
    TheFunction->insert(TheFunction->end(), MergeBB);
    context->Builder->SetInsertPoint(MergeBB);
    llvm::PHINode *PN =
        context->Builder->CreatePHI(llvm::Type::getDoubleTy(*context->TheContext), 2, "iftmp");

    // PN->addIncoming(ThenV, ThenBB);
    // PN->addIncoming(ElseV, ElseBB);
    return PN;
}