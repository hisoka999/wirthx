#include "StringConstantNode.h"
#include "interpreter/Stack.h"
#include <iostream>

StringConstantNode::StringConstantNode(std::string_view literal) : ASTNode(), m_literal(literal)
{
}

void StringConstantNode::print()
{
    std::cout << "\'" << m_literal << "\'";
}

void StringConstantNode::eval(Stack &stack, [[maybe_unused]] std::ostream &outputStream)
{
    stack.push_back(m_literal);
}