#include "VariableAccessNode.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include "FunctionCallNode.h"
#include "MethodCallNode.h"
#include "UnitNode.h"
#include "compare.h"
#include "compiler/Context.h"


VariableAccessNode::VariableAccessNode(const Token &token, bool dereference) :
    ASTNode(token), m_variableName(token.lexical()), m_dereference(dereference)
{
}

void VariableAccessNode::print() { std::cout << m_variableName; }

llvm::Value *VariableAccessNode::codegen(std::unique_ptr<Context> &context)
{
    const auto variableName = to_lower(m_variableName);
    llvm::Value *V = context->namedValue(m_variableName);
    if (V)
    {
        return V;
    }

    llvm::AllocaInst *allocation = context->namedAllocation(m_variableName);


    const auto functionDefinition =
            context->programUnit()->getFunctionDefinition(context->currentFunction()->getName().str());
    if (functionDefinition.has_value() && functionDefinition.value()->parent() &&
        functionDefinition.value()->functionType() != FunctionType::Constructor)
    {
        const auto thisPointer = context->currentFunction()->getArg(0);


        const auto rawType = context->programUnit()
                                     ->getTypeDefinitions()
                                     .getType(functionDefinition.value()->parent().value())
                                     .value();
        if (const auto classType = std::dynamic_pointer_cast<ClassType>(rawType))
        {
            if (const auto member = classType->member(m_variableName); member.has_value())
            {
                const auto fieldName = "self." + m_variableName;
                const auto llvmRecordType = classType->generateLlvmType(context);

                const auto index = classType->getFieldIndexByName(m_variableName);


                const llvm::DataLayout &DL = context->module()->getDataLayout();
                const auto fieldType = member.value().variableDefinition->variableType->generateLlvmType(context);


                const auto fieldPointer =
                        context->builder()->CreateStructGEP(llvmRecordType, thisPointer, index, fieldName);

                const auto alignment = DL.getPrefTypeAlign(fieldType);
                return context->builder()->CreateAlignedLoad(fieldType, fieldPointer, alignment, m_variableName);
            }
        }
    }
    if (!allocation)
    {
        for (auto &arg: context->currentFunction()->args())
        {
            if (iequals(arg.getName(), variableName))
            {

                const auto argType = functionDefinition.value()->getParam(arg.getName().str());
                const auto llvmArgType = argType->type->generateLlvmType(context);
                auto argValue = context->currentFunction()->getArg(arg.getArgNo());
                if (argType->type->baseType == VariableBaseType::Struct)
                {
                    llvm::AllocaInst *alloca =
                            context->builder()->CreateAlloca(llvmArgType, nullptr, argType->argumentName + "_struct");
                    return context->builder()->CreateLoad(allocation->getAllocatedType(), alloca,
                                                          m_variableName.c_str());
                }
                if (argType->isReference && (argType->type->isSimpleType()))
                {

                    return context->builder()->CreateLoad(llvmArgType, argValue);
                }

                return argValue;
            }
        }

        return LogErrorV("Unknown variable name: " + m_variableName);
    }
    // auto type = resolveType(context->programUnit(), resolveParent(context));
    // if (!m_dereference && type->baseType == VariableBaseType::Pointer)
    // {
    //     return A;
    // }

    // Load the value.
    if (allocation->getAllocatedType()->isStructTy() || !context->loadValue)
        return allocation;


    return context->builder()->CreateLoad(allocation->getAllocatedType(), allocation, m_variableName.c_str());
}

std::shared_ptr<VariableType> VariableAccessNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parent)
{
    std::shared_ptr<VariableType> type;
    if (auto *functionDefinition = dynamic_cast<FunctionDefinitionNode *>(parent))
    {
        if (auto param = functionDefinition->getParam(m_variableName))
        {
            type = param.value().type;
        }
        if (auto var = functionDefinition->body()->getVariableDefinition(m_variableName))
        {
            type = var.value().variableType;
        }

        if (type == nullptr && functionDefinition->parent())
        {
            if (auto tmpClassType = unit->getTypeDefinitions().getType(functionDefinition->parent().value()))
            {
                const auto classType = std::dynamic_pointer_cast<ClassType>(tmpClassType.value());
                if (auto memberVariable = classType->member(m_variableName))
                {
                    type = memberVariable.value().variableDefinition->variableType;
                }
            }
        }
    }
    else if (auto *functionCall = dynamic_cast<FunctionCallNode *>(parent))
    {
        if (auto unitFunctionDefinition = unit->getFunctionDefinition(functionCall->name()))
        {
            if (auto param = unitFunctionDefinition.value()->getParam(m_variableName))
            {
                type = param.value().type;
            }
            if (auto var = unitFunctionDefinition.value()->body()->getVariableDefinition(m_variableName))
            {
                type = var.value().variableType;
            }
        }
    }
    else if (auto *methodCall = dynamic_cast<MethodCallNode *>(parent))
    {

        if (auto unitFunctionDefinition = methodCall->memberFunction().functionDefinition)
        {
            if (auto param = unitFunctionDefinition->getParam(m_variableName))
            {
                type = param.value().type;
            }
            if (auto var = unitFunctionDefinition->body()->getVariableDefinition(m_variableName))
            {
                type = var.value().variableType;
            }
        }
    }
    if (type == nullptr)
    {
        if (auto definition = unit->getVariableDefinition(m_variableName))
        {
            type = definition.value().variableType;
        }
    }

    if (m_dereference)
    {
        if (auto ptrType = std::dynamic_pointer_cast<PointerType>(type))
        {
            return ptrType->pointerBase;
        }
    }
    else if (type)
    {
        return type;
    }


    return std::make_shared<VariableType>();
}
