#include "ComparissionNode.h"
#include "interpreter/Stack.h"
#include <iostream>

ComparrisionNode::ComparrisionNode(CMPOperator op, const std::shared_ptr<ASTNode> &lhs,
                                   const std::shared_ptr<ASTNode> &rhs) : m_lhs(lhs), m_rhs(rhs), m_operator(op)
{
}

void ComparrisionNode::print()
{
    m_lhs->print();
    switch (m_operator)
    {
    case CMPOperator::EQUALS:
        std::cout << "=";
        break;
    case CMPOperator::GREATER:
        std::cout << ">";
        break;
    case CMPOperator::GREATER_EQUAL:
        std::cout << ">=";
        break;
    case CMPOperator::LESS:
        std::cout << "<";
        break;
    case CMPOperator::LESS_EQUAL:
        std::cout << "<=";
        break;
    default:
        break;
    }
    m_rhs->print();
}

void compare_int(CMPOperator op, StackObject lhs, StackObject rhs, Stack &stack)
{

    auto lhs_ = std::get<int64_t>(lhs);
    auto rhs_ = std::get<int64_t>(rhs);
    switch (op)
    {
    case CMPOperator::EQUALS:
        stack.push_back(static_cast<int64_t>(lhs_ == rhs_));
        break;
    case CMPOperator::GREATER:
        stack.push_back(static_cast<int64_t>(lhs_ > rhs_));
        break;
    case CMPOperator::GREATER_EQUAL:
        stack.push_back(static_cast<int64_t>(lhs_ >= rhs_));
        break;
    case CMPOperator::LESS:
        stack.push_back(static_cast<int64_t>(lhs_ < rhs_));
        break;
    case CMPOperator::LESS_EQUAL:
        stack.push_back(static_cast<int64_t>(lhs_ <= rhs_));
        break;
    }
}

void compare_str(CMPOperator op, StackObject lhs, StackObject rhs, Stack &stack)
{

    auto lhs_ = std::get<std::string_view>(lhs);
    auto rhs_ = std::get<std::string_view>(rhs);
    switch (op)
    {
    case CMPOperator::EQUALS:
        stack.push_back(static_cast<int64_t>(lhs_ == rhs_));
        break;
    case CMPOperator::GREATER:
        stack.push_back(static_cast<int64_t>(lhs_ > rhs_));
        break;
    case CMPOperator::GREATER_EQUAL:
        stack.push_back(static_cast<int64_t>(lhs_ >= rhs_));
        break;
    case CMPOperator::LESS:
        stack.push_back(static_cast<int64_t>(lhs_ < rhs_));
        break;
    case CMPOperator::LESS_EQUAL:
        stack.push_back(static_cast<int64_t>(lhs_ <= rhs_));
        break;
    }
}
void ComparrisionNode::eval(Stack &stack)
{
    m_lhs->eval(stack);
    m_rhs->eval(stack);
    auto lhs = stack.pop_front();
    auto rhs = stack.pop_front();
    if (std::holds_alternative<int64_t>(lhs) && std::holds_alternative<int64_t>(rhs))
    {
        compare_int(m_operator, lhs, rhs, stack);
    }
    else
    {
        compare_str(m_operator, lhs, rhs, stack);
    }
}