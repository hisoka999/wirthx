#include "VariableAssignmentNode.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>

#include "FunctionCallNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "exceptions/CompilerException.h"

VariableAssignmentNode::VariableAssignmentNode(const Token variableName, const std::shared_ptr<ASTNode> &expression) :
    m_variable(variableName), m_variableName(std::string(m_variable.lexical())), m_expression(expression)
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
    llvm::Value *allocatedValue = context->NamedAllocations[m_variableName];

    llvm::Type *type = nullptr;

    if (!allocatedValue)
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

                        allocatedValue = argValue;
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


    if (!allocatedValue)
        return LogErrorV("Unknown variable name for assignment: " + m_variableName);


    auto expressionResult = m_expression->codegen(context);

    if (type->isIntegerTy() && expressionResult->getType()->isIntegerTy())
    {
        auto targetType = llvm::IntegerType::get(*context->TheContext, type->getIntegerBitWidth());
        if (type->getIntegerBitWidth() != expressionResult->getType()->getIntegerBitWidth())
        {
            expressionResult = context->Builder->CreateIntCast(expressionResult, targetType, true, "lhs_cast");
        }

        context->Builder->CreateStore(expressionResult, allocatedValue);
        return expressionResult;
    }


    if (type->isStructTy() && expressionResult->getType()->isPointerTy())
    {
        auto llvmArgType = type;

        auto memcpyCall = llvm::Intrinsic::getDeclaration(
                context->TheModule.get(), llvm::Intrinsic::memcpy,
                {context->Builder->getPtrTy(), context->Builder->getPtrTy(), context->Builder->getInt64Ty()});
        std::vector<llvm::Value *> memcopyArgs;

        const llvm::DataLayout &DL = context->TheModule->getDataLayout();
        uint64_t structSize = DL.getTypeAllocSize(llvmArgType);


        memcopyArgs.push_back(context->Builder->CreateBitCast(allocatedValue, context->Builder->getPtrTy()));
        memcopyArgs.push_back(context->Builder->CreateBitCast(expressionResult, context->Builder->getPtrTy()));
        memcopyArgs.push_back(context->Builder->getInt64(structSize));
        memcopyArgs.push_back(context->Builder->getFalse());

        context->Builder->CreateCall(memcpyCall, memcopyArgs);

        return expressionResult;
    }

    context->Builder->CreateStore(expressionResult, allocatedValue);
    return expressionResult;
}
void VariableAssignmentNode::typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{

    if (const auto varType = unit->getVariableDefinition(m_variableName))
    {
        const auto expressionType = m_expression->resolveType(unit, parentNode);
        if (*expressionType != *varType.value().variableType)
        {

            throw CompilerException(ParserError{.token = m_variable,
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

                    throw CompilerException(ParserError{.token = m_variable,
                                                        .message = "the type for the variable \"" + m_variableName +
                                                                   "\" is \"" + varType.value().variableType->typeName +
                                                                   "\" but a \"" + expressionType->typeName +
                                                                   "\" was assigned."});
                }
            }
        }
    }
}
