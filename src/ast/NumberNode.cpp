#include "NumberNode.h"
#include "interpreter/Stack.h"
#include <iostream>

NumberNode::NumberNode(int64_t value) : ASTNode(), m_value(value)
{
}

void NumberNode::print()
{
    std::cout << m_value;
}

void NumberNode::eval(Stack &stack, [[maybe_unused]] std::ostream &outputStream)
{
    stack.push_back(m_value);
}