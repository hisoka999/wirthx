#include "FunctionDefinitionNode.h"
#include <iostream>
#include "interpreter/Stack.h"

FunctionDefinitionNode::FunctionDefinitionNode(std::string name, std::vector<FunctionArgument> params, std::vector<std::shared_ptr<ASTNode>> body, bool isProcedure)
    : m_name(name), m_params(params), m_body(body), m_isProcedure(isProcedure)
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
        std::cout << param.argumentName + " :" + param.type.typeName;

        if (i != m_params.size() - 1)
        {
            std::cout << ",";
        }
    }
    std::cout << ")\n";
    for (auto &node : m_body)
    {
        node->print();
    }
    // std::cout << "end;\n";
}

void FunctionDefinitionNode::eval([[maybe_unused]] Stack &stack)
{
    for (auto param : m_params)
    {
        auto value = stack.pop_front();
        stack.set_var(param.argumentName, value);
    }
    for (auto &node : m_body)
    {
        node->eval(stack);
    }
}

std::string &FunctionDefinitionNode::name()
{
    return m_name;
}