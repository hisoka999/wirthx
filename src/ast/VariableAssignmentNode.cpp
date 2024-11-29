#include "VariableAssignmentNode.h"
#include <iostream>
#include "FunctionCallNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

VariableAssignmentNode::VariableAssignmentNode(const std::string_view variableName,
                                               const std::shared_ptr<ASTNode> &expression) :
    m_variableName(variableName), m_expression(expression)
{
}

void VariableAssignmentNode::print()
{
    std::cout << m_variableName << ":=";
    m_expression->print();
    std::cout << ";\n";
}

void VariableAssignmentNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    m_expression->eval(context, outputStream);

    VariableBaseType baseType = VariableBaseType::Unknown;
    if (FunctionCallNode *functionCall = dynamic_cast<FunctionCallNode *>(context.parent))
    {
        auto functionDefinition = context.unit->getFunctionDefinition(functionCall->name());
        if (functionDefinition)
        {
            auto param = functionDefinition.value()->getParam(m_variableName);
            if (param)
            {
                baseType = param.value().type->baseType;
            }
        }
    }
    else
    {
        baseType = context.unit->getVariableDefinition(m_variableName).value().variableType->baseType;
    }


    if (baseType == VariableBaseType::String)
    {
        auto value = context.stack.pop_front<std::string_view>();
        context.stack.set_var(m_variableName, value);
    }
    else
    {
        auto value = context.stack.pop_front<int64_t>();
        context.stack.set_var(m_variableName, value);
    }
}

llvm::Value *VariableAssignmentNode::codegen(std::unique_ptr<Context> &context)
{

    // Look this variable up in the function.
    llvm::AllocaInst *V = context->NamedAllocations[m_variableName];

    if (!V)
        return LogErrorV("Unknown variable name");

    auto result = m_expression->codegen(context);

    if (V->getAllocatedType()->isIntegerTy() && result->getType()->isIntegerTy())
    {
        auto targetType = llvm::IntegerType::get(*context->TheContext, V->getAllocatedType()->getIntegerBitWidth());
        if (V->getAllocatedType()->getIntegerBitWidth() != result->getType()->getIntegerBitWidth())
        {
            result = context->Builder->CreateIntCast(result, targetType, true, "lhs_cast");
        }
    }


    context->Builder->CreateStore(result, V);
    return result;
}
