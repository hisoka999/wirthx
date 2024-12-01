#include "FieldAccessNode.h"
#include "FunctionCallNode.h"
#include "FunctionDefinitionNode.h"
#include "RecordType.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "exceptions/CompilerException.h"


FieldAccessNode::FieldAccessNode(const TokenWithFile element, const TokenWithFile field) :
    m_element(element), m_elementName(element.token.lexical), m_field(field), m_fieldName(field.token.lexical)
{
}

void FieldAccessNode::print() {}

llvm::Value *FieldAccessNode::codegen(std::unique_ptr<Context> &context)
{
    llvm::AllocaInst *V = context->NamedAllocations[m_elementName];

    const auto fieldName = m_elementName + "." + m_fieldName;
    if (!V)
    {
        for (size_t i = 0; i < context->TopLevelFunction->arg_size(); ++i)
        {
            auto arg = context->TopLevelFunction->getArg(i);
            if (arg->getName() == m_elementName)
            {

                auto functionDefinition =
                        context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str());
                auto structDef = functionDefinition.value()->getParam(m_elementName);

                if (!structDef)
                {
                    return LogErrorV("Unknown variable name");
                }

                auto recordType = std::dynamic_pointer_cast<RecordType>(structDef->type);
                auto llvmRecordType = llvm::cast<llvm::StructType>(recordType->generateLlvmType(context));

                auto index = recordType->getFieldIndexByName(m_fieldName);
                auto field = recordType->getField(index);
                llvm::Value *value = arg;
                if (arg->getType()->isStructTy())
                {
                    llvm::AllocaInst *alloca =
                            context->Builder->CreateAlloca(llvmRecordType, nullptr, m_elementName + "_ptr");
                    context->Builder->CreateStore(arg, alloca);

                    value = alloca;
                }
                auto fieldType = field.variableType->generateLlvmType(context);
                const llvm::DataLayout &DL = context->TheModule->getDataLayout();
                auto alignment = DL.getPrefTypeAlign(fieldType);


                auto arrayValue = context->Builder->CreateStructGEP(llvmRecordType, value, index, fieldName);
                return context->Builder->CreateAlignedLoad(fieldType, arrayValue, alignment, fieldName);
            }
        }
    }
    std::optional<VariableDefinition> structDef;
    if (context->TopLevelFunction)
    {
        auto functionDefinition =
                context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str());
        if (functionDefinition)
            structDef = functionDefinition.value()->body()->getVariableDefinition(m_elementName);
    }

    if (!structDef)
    {
        structDef = context->ProgramUnit->getVariableDefinition(m_elementName);
    }
    if (!structDef)
    {
        return LogErrorV("Unknown record variable name");
    }
    else
    {
        auto recordType = std::dynamic_pointer_cast<RecordType>(structDef->variableType);

        auto index = recordType->getFieldIndexByName(m_fieldName);
        auto field = recordType->getField(index);


        auto arrayValue = context->Builder->CreateStructGEP(V->getAllocatedType(), V, index, fieldName);
        return context->Builder->CreateLoad(field.variableType->generateLlvmType(context), arrayValue);
    }
}

std::shared_ptr<VariableType> FieldAccessNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    if (auto functionDefinition = dynamic_cast<FunctionDefinitionNode *>(parentNode))
    {
        auto param = functionDefinition->getParam(m_elementName);
        if (param)
        {
            if (param->type->baseType == VariableBaseType::Struct)
            {
                auto type = std::dynamic_pointer_cast<RecordType>(param->type);

                auto field = type->getFieldByName(m_fieldName);

                if (field)
                {
                    return field.value().variableType;
                }
            }
        }
        auto variable = functionDefinition->body()->getVariableDefinition(m_elementName);
        if (variable)
        {
            if (variable->variableType->baseType == VariableBaseType::Struct)
            {
                auto type = std::dynamic_pointer_cast<RecordType>(variable->variableType);

                auto field = type->getFieldByName(m_fieldName);

                if (field)
                {
                    return field.value().variableType;
                }
            }
        }
    }
    auto definition = unit->getVariableDefinition(m_elementName);

    if (definition->variableType->baseType == VariableBaseType::Struct)
    {
        auto type = std::dynamic_pointer_cast<RecordType>(definition->variableType);

        auto field = type->getFieldByName(m_fieldName);

        if (field)
        {
            return field.value().variableType;
        }
    }

    return std::make_shared<VariableType>();
}
