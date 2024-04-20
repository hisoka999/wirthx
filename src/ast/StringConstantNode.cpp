#include "StringConstantNode.h"
#include <iostream>
#include "interpreter/Stack.h"

StringConstantNode::StringConstantNode(std::string_view literal) : ASTNode(), m_literal(literal)
{
}

void StringConstantNode::print()
{
    std::cout << "\'" << m_literal << "\'";
}

void StringConstantNode::eval(Stack &stack)
{
    stack.push_back(m_literal);
}