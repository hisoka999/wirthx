#include "PrintNode.h"
#include <iostream>
#include "interpreter/Stack.h"
#include <variant>

PrintNode::PrintNode(std::vector<std::shared_ptr<ASTNode>> &args) : ASTNode(), m_args(args) {}

void PrintNode::print()
{
    std::cout << "print ";
    for (auto &c : m_args)
    {
        c->print();
    }
    std::cout << "\n";
}

void PrintNode::eval(Stack &stack)
{
    for (auto &arg : m_args)
    {
        arg->eval(stack);
    }
    // todo get elements from stack
    auto value = stack.pop_front();
    if (std::holds_alternative<int64_t>(value))
    {
        std::cout << std::get<int64_t>(value) << "\n";
    }
    else
    {
        std::cout << std::get<std::string_view>(value) << "\n";
    }
}