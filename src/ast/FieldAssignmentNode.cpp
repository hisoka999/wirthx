#include "FieldAssignmentNode.h"
#include "RecordType.h"
#include "UnitNode.h"
#include "compiler/Context.h"

FieldAssignmentNode::FieldAssignmentNode(const TokenWithFile variable, const TokenWithFile field,
                                         const std::shared_ptr<ASTNode> &expression) :
    m_variable(std::move(variable)), m_variableName(std::string(m_variable.token.lexical)), m_field(std::move(field)),
    m_fieldName(std::string(m_field.token.lexical)), m_expression(expression)
{
}

void FieldAssignmentNode::print() {}

void FieldAssignmentNode::eval(InterpreterContext &context, std::ostream &outputStream) {}

llvm::Value *FieldAssignmentNode::codegen(std::unique_ptr<Context> &context)
{
    using namespace std::string_literals;
    llvm::AllocaInst *V = context->NamedAllocations[m_variableName];

    const auto fieldName = m_variableName + "." + m_fieldName;

    if (!V)
    {
        for (size_t i = 0; i < context->TopLevelFunction->arg_size(); ++i)
        {
            auto arg = context->TopLevelFunction->getArg(i);
            if (arg->getName() == m_variableName)
            {
                auto functionDefinition =
                        context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str());
                auto structDef = functionDefinition.value()->getParam(m_variableName);

                if (!structDef)
                {
                    return LogErrorV("Unknown variable name");
                }

                auto recordType = std::dynamic_pointer_cast<RecordType>(structDef->type);
                auto llvmRecordType = llvm::cast<llvm::StructType>(recordType->generateLlvmType(context));

                auto index = recordType->getFieldIndexByName(m_fieldName);
                auto field = recordType->getField(index);
                auto result = m_expression->codegen(context);

                if (arg->getType()->isStructTy())
                {
                    llvm::AllocaInst *alloca =
                            context->Builder->CreateAlloca(llvmRecordType, nullptr, m_variableName + "_ptr");
                    context->Builder->CreateStore(arg, alloca);

                    auto arrayValue = context->Builder->CreateStructGEP(llvmRecordType, alloca, index, fieldName);
                    context->Builder->CreateStore(result, arrayValue);
                }
                else
                {
                    auto arrayValue = context->Builder->CreateStructGEP(llvmRecordType, arg, index, fieldName);
                    context->Builder->CreateStore(result, arrayValue);
                }
                return result;
            }
        }
    }
    if (!V)
        return LogErrorV("Unknown record variable name "s + m_variableName);


    std::optional<VariableDefinition> structDef;
    if (context->TopLevelFunction)
    {
        auto functionDefinition =
                context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str());
        if (functionDefinition)
            structDef = functionDefinition.value()->body()->getVariableDefinition(m_variableName);
    }

    if (!structDef)
    {
        structDef = context->ProgramUnit->getVariableDefinition(m_variableName);
    }
    auto recordType = std::dynamic_pointer_cast<RecordType>(structDef->variableType);

    auto index = recordType->getFieldIndexByName(m_fieldName);
    auto field = recordType->getField(index);


    auto elementPointer = context->Builder->CreateStructGEP(recordType->generateLlvmType(context), V, index, fieldName);

    auto result = m_expression->codegen(context);
    const llvm::DataLayout &DL = context->TheModule->getDataLayout();
    auto alignment = DL.getPrefTypeAlign(field.variableType->generateLlvmType(context));

    context->Builder->CreateAlignedStore(result, elementPointer, alignment);
    return result;
}
