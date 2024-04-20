#include "VariableAssignmentNode.h"
#include "interpreter/Stack.h"
#include <iostream>

VariableAssignmentNode::VariableAssignmentNode(const std::string_view variableName, const std::shared_ptr<ASTNode> &expression) : m_variableName(variableName), m_expression(expression)
{
}

void VariableAssignmentNode::print()
{
    std::cout << m_variableName << ":=";
    m_expression->print();
    std::cout << ";\n";
}

void VariableAssignmentNode::eval(Stack &stack)
{
    m_expression->eval(stack);
    auto value = stack.pop_front();
    stack.set_var(m_variableName, value);
}