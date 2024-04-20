#include "LogicalExpressionNode.h"
#include "interpreter/Stack.h"
#include <iostream>

LogicalExpressionNode::LogicalExpressionNode(LogicalOperator op, const std::shared_ptr<ASTNode> &lhs,
                                             const std::shared_ptr<ASTNode> &rhs) : m_lhs(lhs), m_rhs(rhs), m_operator(op)
{
}

LogicalExpressionNode::LogicalExpressionNode(LogicalOperator op,
                                             const std::shared_ptr<ASTNode> &rhs) : m_lhs(nullptr), m_rhs(rhs), m_operator(op)
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

void LogicalExpressionNode::eval(Stack &stack)
{

    switch (m_operator)
    {
    case LogicalOperator::AND:
    {
        m_lhs->eval(stack);
        auto lhs = stack.pop_front<int64_t>();
        m_rhs->eval(stack);
        auto rhs = stack.pop_front<int64_t>();

        stack.push_back(static_cast<int64_t>(lhs && rhs));
    }
    break;
    case LogicalOperator::OR:
    {
        m_lhs->eval(stack);
        auto lhs = stack.pop_front<int64_t>();
        m_rhs->eval(stack);
        auto rhs = stack.pop_front<int64_t>();

        stack.push_back(static_cast<int64_t>(lhs || rhs));
    }
    break;
    case LogicalOperator::NOT:
    {
        m_rhs->eval(stack);
        auto rhs = stack.pop_front<int64_t>();
        stack.push_back(static_cast<int64_t>(!rhs));
    }
    break;
    default:
        break;
    }
}