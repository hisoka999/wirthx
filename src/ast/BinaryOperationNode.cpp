#include "BinaryOperationNode.h"
#include <iostream>
#include "interpreter/Stack.h"

BinaryOperationNode::BinaryOperationNode(Operator op, const std::shared_ptr<ASTNode> &lhs,
                                         const std::shared_ptr<ASTNode> &rhs) : ASTNode(), m_lhs(lhs), m_rhs(rhs), m_operator(op)
{
}

void BinaryOperationNode::print()
{
    m_lhs->print();
    std::cout << static_cast<char>(m_operator);
    m_rhs->print();
}

void BinaryOperationNode::eval(Stack &stack)
{
    m_lhs->eval(stack);
    m_rhs->eval(stack);
    auto lhs = stack.pop_front<int64_t>();
    auto rhs = stack.pop_front<int64_t>();
    switch (m_operator)
    {
    case Operator::PLUS:
        stack.push_back(lhs + rhs);
        break;
    case Operator::MINUS:
        stack.push_back(lhs - rhs);
        break;
    case Operator::MUL:
        stack.push_back(lhs * rhs);
        break;
    }
}