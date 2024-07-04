#include "PrintNode.h"
#include "interpreter/Stack.h"
#include <iostream>
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

void PrintNode::eval(Stack &stack, std::ostream &outputStream)
{
    for (auto &arg : m_args)
    {
        arg->eval(stack, outputStream);
    }
    // todo get elements from stack
    auto value = stack.pop_front();
    if (std::holds_alternative<int64_t>(value))
    {
        outputStream << std::get<int64_t>(value) << "\n";
    }
    else
    {
        outputStream << std::get<std::string_view>(value) << "\n";
    }
}

llvm::Value *PrintNode::codegen(std::unique_ptr<Context> &context)
{
    return nullptr;
}