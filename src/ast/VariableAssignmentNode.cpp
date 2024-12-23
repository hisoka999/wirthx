#include "VariableAssignmentNode.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>

#include "FunctionCallNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "exceptions/CompilerException.h"

VariableAssignmentNode::VariableAssignmentNode(const TokenWithFile variableName,
                                               const std::shared_ptr<ASTNode> &expression) :
    m_variable(variableName), m_variableName(std::string(m_variable.token.lexical)), m_expression(expression)
{
}

void VariableAssignmentNode::print()
{
    std::cout << m_variableName << ":=";
    m_expression->print();
    std::cout << ";\n";
}

llvm::Value *VariableAssignmentNode::codegen(std::unique_ptr<Context> &context)
{
    // Look this variable up in the function.
    llvm::Value *V = context->NamedAllocations[m_variableName];

    llvm::Type *type = nullptr;

    if (!V)
    {
        for (auto &arg: context->TopLevelFunction->args())
        {
            if (arg.getName() == m_variableName)
            {
                auto functionDefinition =
                        context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str());
                if (functionDefinition.has_value())
                {
                    const auto argType = functionDefinition.value()->getParam(arg.getArgNo());
                    type = argType->type->generateLlvmType(context);
                    const auto argValue = context->TopLevelFunction->getArg(arg.getArgNo());
                    if (argType->isReference)
                    {

                        V = argValue;
                        break;
                    }
                }
            }
        }
    }
    else
    {
        type = context->NamedAllocations[m_variableName]->getAllocatedType();
    }


    if (!V)
        return LogErrorV("Unknown variable name for assignment: " + m_variableName);


    auto result = m_expression->codegen(context);

    if (type->isIntegerTy() && result->getType()->isIntegerTy())
    {
        auto targetType = llvm::IntegerType::get(*context->TheContext, type->getIntegerBitWidth());
        if (type->getIntegerBitWidth() != result->getType()->getIntegerBitWidth())
        {
            result = context->Builder->CreateIntCast(result, targetType, true, "lhs_cast");
        }

        context->Builder->CreateStore(result, V);
        return result;
    }


    if (type->isStructTy() && result->getType()->isPointerTy())
    {
        auto llvmArgType = type;

        auto memcpyCall = llvm::Intrinsic::getDeclaration(
                context->TheModule.get(), llvm::Intrinsic::memcpy,
                {context->Builder->getPtrTy(), context->Builder->getPtrTy(), context->Builder->getInt64Ty()});
        std::vector<llvm::Value *> memcopyArgs;
        // llvm::AllocaInst *alloca = context->Builder->CreateAlloca(llvmArgType, nullptr, m_variableName + "_ptr");

        const llvm::DataLayout &DL = context->TheModule->getDataLayout();
        uint64_t structSize = DL.getTypeAllocSize(llvmArgType);


        memcopyArgs.push_back(context->Builder->CreateBitCast(V, context->Builder->getPtrTy()));
        memcopyArgs.push_back(context->Builder->CreateBitCast(result, context->Builder->getPtrTy()));
        memcopyArgs.push_back(context->Builder->getInt64(structSize));
        memcopyArgs.push_back(context->Builder->getFalse());

        context->Builder->CreateCall(memcpyCall, memcopyArgs);

        return result;
    }

    context->Builder->CreateStore(result, V);
    return result;
}
void VariableAssignmentNode::typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{

    if (const auto varType = unit->getVariableDefinition(m_variableName))
    {
        const auto expressionType = m_expression->resolveType(unit, parentNode);
        if (*expressionType != *varType.value().variableType)
        {

            throw CompilerException(ParserError{.file_name = m_variable.fileName,
                                                .token = m_variable.token,
                                                .message = "the type for the variable \"" + m_variableName +
                                                           "\" is \"" + varType.value().variableType->typeName +
                                                           "\" but a \"" + expressionType->typeName +
                                                           "\" was assigned."});
        }
    }

    if (parentNode != unit.get())
    {
        if (const auto functionDef = dynamic_cast<FunctionDefinitionNode *>(parentNode))
        {
            if (const auto varType = functionDef->body()->getVariableDefinition(m_variableName))
            {
                const auto expressionType = m_expression->resolveType(unit, parentNode);
                if (*expressionType != *varType.value().variableType)
                {

                    throw CompilerException(ParserError{.file_name = m_variable.fileName,
                                                        .token = m_variable.token,
                                                        .message = "the type for the variable \"" + m_variableName +
                                                                   "\" is \"" + varType.value().variableType->typeName +
                                                                   "\" but a \"" + expressionType->typeName +
                                                                   "\" was assigned."});
                }
            }
        }
    }
}
