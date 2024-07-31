#include "PrintNode.h"
#include <iostream>
#include <variant>
#include "interpreter/InterpreterContext.h"

PrintNode::PrintNode(std::vector<std::shared_ptr<ASTNode>> &args) : ASTNode(), m_args(args) {}

void PrintNode::print()
{
    std::cout << "print ";
    for (auto &c: m_args)
    {
        c->print();
    }
    std::cout << "\n";
}

void PrintNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    VariableBaseType baseType;
    for (auto &arg: m_args)
    {
        arg->eval(context, outputStream);
        baseType = arg->resolveType(context.unit, context.parent)->baseType;
    }
    // todo get elements from stack

    if (baseType == VariableBaseType::Integer)
    {
        auto value = context.stack.pop_front<int64_t>();
        outputStream << value << "\n";
    }
    else
    {
        auto value = context.stack.pop_front<std::string_view>();
        outputStream << value << "\n";
    }
}

llvm::Value *PrintNode::codegen([[maybe_unused]] std::unique_ptr<Context> &context) { return nullptr; }
