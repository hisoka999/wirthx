#include "FunctionDefinitionNode.h"
#include "interpreter/Stack.h"
#include <iostream>

FunctionDefinitionNode::FunctionDefinitionNode(std::string name, std::vector<FunctionArgument> params, std::vector<std::shared_ptr<ASTNode>> body, bool isProcedure, VariableType returnType)
    : m_name(name), m_params(params), m_body(body), m_isProcedure(isProcedure), m_returnType(returnType)
{
}

void FunctionDefinitionNode::print()
{
    if (m_isProcedure)
        std::cout << "procedure " + m_name + "(";
    else
        std::cout << "function " + m_name + "(";
    for (size_t i = 0; i < m_params.size(); ++i)
    {
        auto &param = m_params[i];
        if (param.isReference)
        {
            std::cout << "var ";
        }
        std::cout << param.argumentName + " :" + param.type.typeName;

        if (i != m_params.size() - 1)
        {
            std::cout << ",";
        }
    }
    std::cout << ")";
    if (m_isProcedure)
    {
        std::cout << "\n";
    }
    else
    {
        std::cout << ": " << m_returnType.typeName << ";\n";
    }
    for (auto &node : m_body)
    {
        node->print();
    }
    // std::cout << "end;\n";
}

void FunctionDefinitionNode::eval(Stack &stack, std::ostream &outputStream)
{
    for (auto param : m_params)
    {
        auto value = stack.pop_front();
        stack.set_var(param.argumentName, value);
    }
    for (auto &node : m_body)
    {
        node->eval(stack, outputStream);
    }
}

std::string &FunctionDefinitionNode::name()
{
    return m_name;
}