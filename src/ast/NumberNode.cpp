#include "NumberNode.h"
#include <iostream>
#include "interpreter/Stack.h"

NumberNode::NumberNode(int64_t value) : ASTNode(), m_value(value)
{
}

void NumberNode::print()
{
    std::cout << m_value;
}

void NumberNode::eval(Stack &stack)
{
    stack.push_back(m_value);
}