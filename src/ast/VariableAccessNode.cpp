#include "VariableAccessNode.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>

#include "FunctionCallNode.h"
#include "UnitNode.h"
#include "compare.h"
#include "compiler/Context.h"


VariableAccessNode::VariableAccessNode(const Token &token) : ASTNode(token), m_variableName(token.lexical()) {}

void VariableAccessNode::print() { std::cout << m_variableName; }

llvm::Value *VariableAccessNode::codegen(std::unique_ptr<Context> &context)
{
    auto variableName = to_lower(m_variableName);
    llvm::Value *V = context->NamedValues[m_variableName];
    if (V)
    {
        return V;
    }

    llvm::AllocaInst *A = context->NamedAllocations[m_variableName];

    if (!A)
    {
        for (auto &arg: context->TopLevelFunction->args())
        {
            if (iequals(arg.getName(), variableName))
            {
                auto functionDefinition =
                        context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str());

                const auto argType = functionDefinition.value()->getParam(arg.getArgNo());
                const auto llvmArgType = argType->type->generateLlvmType(context);
                auto argValue = context->TopLevelFunction->getArg(arg.getArgNo());
                if (argType->type->baseType == VariableBaseType::Struct)
                {
                    llvm::AllocaInst *alloca =
                            context->Builder->CreateAlloca(llvmArgType, nullptr, argType->argumentName + "_struct");
                    return context->Builder->CreateLoad(A->getAllocatedType(), alloca, m_variableName.c_str());
                }
                if (argType->isReference && (argType->type->isSimpleType()))
                {

                    return context->Builder->CreateLoad(llvmArgType, argValue);
                }

                return argValue;
            }
        }


        return LogErrorV("Unknown variable name: " + m_variableName);
    }
    // Load the value.
    if (A->getAllocatedType()->isStructTy() || !context->loadValue)
        return A;


    return context->Builder->CreateLoad(A->getAllocatedType(), A, m_variableName.c_str());
}

std::shared_ptr<VariableType> VariableAccessNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parent)
{
    if (auto *functionDefinition = dynamic_cast<FunctionDefinitionNode *>(parent))
    {
        if (auto param = functionDefinition->getParam(m_variableName))
        {
            return param.value().type;
        }
        if (auto var = functionDefinition->body()->getVariableDefinition(m_variableName))
        {
            return var.value().variableType;
        }
    }

    if (auto *functionCall = dynamic_cast<FunctionCallNode *>(parent))
    {
        if (auto functionDefinition = unit->getFunctionDefinition(functionCall->name()))
        {
            if (auto param = functionDefinition.value()->getParam(m_variableName))
            {
                return param.value().type;
            }
            auto var = functionDefinition.value()->body()->getVariableDefinition(m_variableName);
            if (var)
            {
                return var.value().variableType;
            }
        }
    }

    if (auto definition = unit->getVariableDefinition(m_variableName))
    {
        return definition.value().variableType;
    }
    return std::make_shared<VariableType>();
}
