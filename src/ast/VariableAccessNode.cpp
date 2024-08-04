#include "VariableAccessNode.h"
#include <iostream>
#include "FunctionCallNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

VariableAccessNode::VariableAccessNode(const std::string_view variableName) : m_variableName(variableName) {}

void VariableAccessNode::print() { std::cout << m_variableName; }

void VariableAccessNode::eval(InterpreterContext &context, [[maybe_unused]] std::ostream &outputStream)
{
    VariableBaseType baseType = VariableBaseType::Unknown;

    if (context.parent != nullptr)
    {
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
    }
    else
    {
        baseType = context.unit->getVariableDefinition(m_variableName).value().variableType->baseType;
    }

    if (baseType == VariableBaseType::String)
    {
        context.stack.push_back(context.stack.get_var<std::string_view>(m_variableName));
    }
    else if (baseType == VariableBaseType::Integer)
    {
        context.stack.push_back(context.stack.get_var<int64_t>(m_variableName));
    }
}

llvm::Value *VariableAccessNode::codegen(std::unique_ptr<Context> &context)
{
    llvm::AllocaInst *A = context->NamedAllocations[m_variableName];

    if (!A)
    {
        for (auto &arg: context->TopLevelFunction->args())
        {
            if (arg.getName() == m_variableName)
            {
                return context->TopLevelFunction->getArg(arg.getArgNo());
            }
        }

        llvm::Value *V = context->NamedValues[m_variableName];
        if (V)
        {
            return V;
        }

        return LogErrorV("Unknown variable name");
    }

    // Load the value.
    return context->Builder->CreateLoad(A->getAllocatedType(), A, m_variableName.c_str());
}

std::shared_ptr<VariableType> VariableAccessNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parent)
{
    if (FunctionDefinitionNode *functionDefinition = dynamic_cast<FunctionDefinitionNode *>(parent))
    {
        auto param = functionDefinition->getParam(m_variableName);
        if (param)
        {
            return param.value().type;
        }
    }

    if (FunctionCallNode *functionCall = dynamic_cast<FunctionCallNode *>(parent))
    {
        auto functionDefinition = unit->getFunctionDefinition(functionCall->name());
        if (functionDefinition)
        {
            auto param = functionDefinition.value()->getParam(m_variableName);
            if (param)
            {
                return param.value().type;
            }
        }
    }

    auto definition = unit->getVariableDefinition(m_variableName);
    if (definition)
    {
        return definition.value().variableType;
    }
    return std::make_shared<VariableType>();
}
