#include "FunctionCallNode.h"
#include <iostream>
#include "interpreter/Stack.h"
#include "FunctionDefinitionNode.h"

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

void FunctionCallNode::eval(Stack &stack)
{
    for (auto &arg : m_args)
    {
        arg->eval(stack);
    }
    auto &func = stack.getFunction(m_name);
    func->eval(stack);
}