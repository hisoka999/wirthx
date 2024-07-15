#include "VariableAccessNode.h"
#include "compiler/Context.h"
#include "interpreter/Stack.h"
#include <iostream>

VariableAccessNode::VariableAccessNode(const std::string_view variableName) : m_variableName(variableName)
{
}

void VariableAccessNode::print()
{
    std::cout << m_variableName;
}

void VariableAccessNode::eval([[maybe_unused]] Stack &stack, [[maybe_unused]] std::ostream &outputStream)
{

    stack.push_back(stack.get_var(m_variableName));
}

llvm::Value *VariableAccessNode::codegen(std::unique_ptr<Context> &context)
{
    llvm::AllocaInst *A = context->NamedValues[m_variableName];

    if (!A)
    {
        for (auto &arg : context->TopLevelFunction->args())
        {
            if (arg.getName() == m_variableName)
            {
                return context->TopLevelFunction->getArg(arg.getArgNo());
            }
        }
        return LogErrorV("Unknown variable name");
    }

    // Load the value.
    return context->Builder->CreateLoad(A->getAllocatedType(), A, m_variableName.c_str());
}

VariableType VariableAccessNode::resolveType(std::unique_ptr<Context> &context)
{
    llvm::AllocaInst *A = context->NamedValues[m_variableName];

    llvm::Type *type;
    if (A)
    {
        type = A->getAllocatedType();
    }
    else
    {
        for (auto &arg : context->TopLevelFunction->args())
        {
            if (arg.getName() == m_variableName)
            {
                type = arg.getType();
                break;
            }
        }
    }
    if (type->isIntegerTy())
    {
        return VariableType::getInteger();
    }
    else if (type->isPointerTy())
    {
        return VariableType::getString();
    }
    else
    {
    }

    return VariableType{};
}