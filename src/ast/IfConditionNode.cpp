#include "IfConditionNode.h"
#include <iostream>
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

IfConditionNode::IfConditionNode(std::shared_ptr<ASTNode> conditionNode,
                                 std::vector<std::shared_ptr<ASTNode>> ifExpressions,
                                 std::vector<std::shared_ptr<ASTNode>> elseExpressions) :
    m_conditionNode(conditionNode), m_ifExpressions(ifExpressions), m_elseExpressions(elseExpressions)
{
}

void IfConditionNode::print()
{
    std::cout << "if ";
    m_conditionNode->print();
    std::cout << " then\n";

    for (auto &exp: m_ifExpressions)
    {
        exp->print();
    }
    if (m_elseExpressions.size() > 0)
    {
        std::cout << "else\n";
        for (auto &exp: m_elseExpressions)
        {
            exp->print();
        }
        // std::cout << "end;\n";
    }
}

void IfConditionNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    m_conditionNode->eval(context, outputStream);
    // check result
    auto result = context.stack.pop_front<int64_t>();
    if (result)
    {
        for (auto &exp: m_ifExpressions)
        {
            exp->eval(context, outputStream);
        }
    }
    else
    {
        for (auto &exp: m_elseExpressions)
        {
            exp->eval(context, outputStream);
        }
    }
}

llvm::Value *IfConditionNode::codegenIf(std::unique_ptr<Context> &context)
{
    llvm::Value *CondV = m_conditionNode->codegen(context);
    if (!CondV)
        return nullptr;
    CondV = context->Builder->CreateICmpEQ(CondV, context->Builder->getInt1(1), "ifcond");

    llvm::Function *TheFunction = context->Builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*context->TheContext, "then", TheFunction);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*context->TheContext, "ifcont");

    context->Builder->CreateCondBr(CondV, ThenBB, MergeBB);

    context->Builder->SetInsertPoint(ThenBB);

    for (auto &exp: m_ifExpressions)
    {
        exp->codegen(context);
    }
    if (!context->BreakBlock.BlockUsed)
        context->Builder->CreateBr(MergeBB);
    TheFunction->insert(TheFunction->end(), MergeBB);
    context->Builder->SetInsertPoint(MergeBB);

    return CondV;
}

llvm::Value *IfConditionNode::codegenIfElse(std::unique_ptr<Context> &context)
{
    llvm::Value *CondV = m_conditionNode->codegen(context);
    if (!CondV)
        return nullptr;

    // Convert condition to a bool by comparing non-equal to 0.0.
    CondV = context->Builder->CreateICmpEQ(CondV, context->Builder->getInt1(1), "ifcond");
    llvm::Function *TheFunction = context->Builder->GetInsertBlock()->getParent();

    // Create blocks for the then and else cases.  Insert the 'then' block at the
    // end of the function.
    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(*context->TheContext, "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*context->TheContext, "else");
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*context->TheContext, "ifcont");
    context->Builder->CreateCondBr(CondV, ThenBB, ElseBB);

    // Emit then value.
    context->Builder->SetInsertPoint(ThenBB);

    for (auto &exp: m_ifExpressions)
    {
        exp->codegen(context);
    }
    if (!context->BreakBlock.BlockUsed)
        context->Builder->CreateBr(MergeBB);
    // Codegen of 'Then' can change the current block, update ThenBB for the PHI.
    ThenBB = context->Builder->GetInsertBlock();

    // Emit else block.
    TheFunction->insert(TheFunction->end(), ElseBB);
    context->Builder->SetInsertPoint(ElseBB);

    for (auto &exp: m_elseExpressions)
    {
        exp->codegen(context);
    }
    if (!context->BreakBlock.BlockUsed)
        context->Builder->CreateBr(MergeBB);
    // Codegen of 'Else' can change the current block, update ElseBB for the PHI.
    ElseBB = context->Builder->GetInsertBlock();

    // Emit merge block.
    TheFunction->insert(TheFunction->end(), MergeBB);
    context->Builder->SetInsertPoint(MergeBB);
    // llvm::PHINode *PN =
    //     context->Builder->CreatePHI(llvm::Type::getInt64Ty(*context->TheContext), 2, "iftmp");

    // PN->addIncoming(ThenV, ThenBB);
    // PN->addIncoming(ElseV, ElseBB);
    // return PN;
    return nullptr;
}
llvm::Value *IfConditionNode::codegen(std::unique_ptr<Context> &context)
{
    if (m_elseExpressions.size() > 0)
        return codegenIfElse(context);
    else
        return codegenIf(context);
}
