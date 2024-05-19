#include "FunctionCallNode.h"
#include "FunctionDefinitionNode.h"
#include "interpreter/Stack.h"
#include <iostream>

FunctionCallNode::FunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args) : m_name(name), m_args(args)
{
}

void FunctionCallNode::print()
{
    std::cout << m_name << "(";
    for (auto &arg : m_args)
    {
        arg->print();
        std::cout << ",";
    }
    std::cout << ");\n";
}

void FunctionCallNode::eval(Stack &stack, std::ostream &outputStream)
{
    for (auto &arg : m_args)
    {
        arg->eval(stack, outputStream);
    }
    auto &func = stack.getFunction(m_name);
    func->eval(stack, outputStream);
    if (stack.has_var(m_name))
        stack.push_back(stack.get_var(m_name));
}