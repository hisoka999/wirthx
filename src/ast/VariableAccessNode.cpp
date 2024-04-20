#include "VariableAccessNode.h"
#include "interpreter/Stack.h"
#include <iostream>

VariableAccessNode::VariableAccessNode(const std::string_view variableName) : m_variableName(variableName)
{
}

void VariableAccessNode::print()
{
    std::cout << m_variableName;
}

void VariableAccessNode::eval([[maybe_unused]] Stack &stack)
{

    stack.push_back(stack.get_var(m_variableName));
}