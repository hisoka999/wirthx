#include "LogicalExpressionNode.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>

#include "compiler/Context.h"


LogicalExpressionNode::LogicalExpressionNode(LogicalOperator op, const std::shared_ptr<ASTNode> &lhs,
                                             const std::shared_ptr<ASTNode> &rhs) :
    m_lhs(lhs), m_rhs(rhs), m_operator(op)
{
}

LogicalExpressionNode::LogicalExpressionNode(LogicalOperator op, const std::shared_ptr<ASTNode> &rhs) :
    m_lhs(nullptr), m_rhs(rhs), m_operator(op)
{
}

void LogicalExpressionNode::print()
{
    switch (m_operator)
    {
        case LogicalOperator::AND:
        {
            m_lhs->print();
            std::cout << " and ";
            m_rhs->print();
        }
        break;
        case LogicalOperator::OR:
        {
            m_lhs->print();
            std::cout << " or ";
            m_rhs->print();
        }
        break;
        case LogicalOperator::NOT:
        {

            std::cout << " not ";
            m_rhs->print();
        }
        break;
        default:
            break;
    }
}

llvm::Value *LogicalExpressionNode::codegen(std::unique_ptr<Context> &context)
{

    switch (m_operator)
    {
        case LogicalOperator::AND:
            return context->Builder->CreateAnd(m_lhs->codegen(context), m_rhs->codegen(context));
        case LogicalOperator::OR:
            return context->Builder->CreateOr(m_lhs->codegen(context), m_rhs->codegen(context));
        case LogicalOperator::NOT:
            return context->Builder->CreateNot(m_rhs->codegen(context));
        default:
            break;
    }
    return nullptr;
}
