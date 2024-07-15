#include "VariableAssignmentNode.h"
#include "compiler/Context.h"
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

void VariableAssignmentNode::eval(Stack &stack, std::ostream &outputStream)
{
    m_expression->eval(stack, outputStream);
    auto value = stack.pop_front();
    stack.set_var(m_variableName, value);
}

llvm::Value *VariableAssignmentNode::codegen(std::unique_ptr<Context> &context)
{

    // Look this variable up in the function.
    llvm::AllocaInst *V = context->NamedValues[m_variableName];
    if (!V)
    {
        if (context->TopLevelFunction->getName() == m_variableName)
        {
            auto result = m_expression->codegen(context);

            return context->Builder->CreateRet(result);
        }
    }

    if (!V)
        return LogErrorV("Unknown variable name");

    auto result = m_expression->codegen(context);

    context->Builder->CreateStore(result, V);
    return result;
}