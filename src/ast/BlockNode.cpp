#include "BlockNode.h"
#include <iostream>

BlockNode::BlockNode(std::vector<VariableDefinition> variableDefinitions, const std::vector<std::shared_ptr<ASTNode>> &expressions) : m_expressions(expressions), m_variableDefinitions(variableDefinitions)
{
}

void BlockNode::print()
{
    if (!m_variableDefinitions.empty())
        std::cout << "var\n";
    for (auto &def : m_variableDefinitions)
    {
        std::cout << def.variableName << " : " << def.variableType.typeName << ";\n";
    }
    std::cout << "begin\n";
    for (auto exp : m_expressions)
    {
        exp->print();
    }

    std::cout << "end;\n";
}

void BlockNode::eval(Stack &stack, std::ostream &outputStream)
{
    for (auto &exp : m_expressions)
    {
        exp->eval(stack, outputStream);
    }
}