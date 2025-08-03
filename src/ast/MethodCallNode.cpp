//
// Created by stefan on 26.07.25.
//

#include "MethodCallNode.h"

#include <cassert>
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>

#include "FunctionCallNode.h"
#include "FunctionDefinitionNode.h"
#include "UnitNode.h"
#include "VariableAccessNode.h"
#include "compare.h"
#include "compiler/Context.h"
MethodCallNode::MethodCallNode(const Token &token, Token methodName, MemberFunction memberFunction,
                               std::vector<std::shared_ptr<ASTNode>> arguments) :
    ASTNode(token), m_methodName(std::move(methodName)), m_memberFunction(std::move(memberFunction)),
    m_arguments(std::move(arguments))
{
}
void MethodCallNode::print() {}

std::string MethodCallNode::callSignature(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    ASTNode *parent = unit.get();
    if (parentNode != nullptr)
    {
        parent = parentNode;
    }
    auto token = expressionToken();
    auto type = to_lower(unit->getVariableDefinition(token.lexical())->variableType->typeName);

    std::string result = type + "." + to_lower(m_methodName.lexical()) + "(";
    for (size_t i = 0; i < m_arguments.size(); ++i)
    {
        const auto arg = m_arguments.at(i)->resolveType(unit, parent);

        result += arg->typeName + ((i < m_arguments.size() - 1) ? "," : "");
    }
    result += ")";
    return result;
}

llvm::Value *MethodCallNode::codegen(std::unique_ptr<Context> &context)
{
    auto m_name = m_methodName.lexical();


    // Look up the name in the global module table.
    ASTNode *parent = resolveParent(context);

    std::string functionName = callSignature(context->programUnit(), parent);

    auto functionDefinition = m_memberFunction.functionDefinition;
    llvm::Function *CalleeF = context->module()->getFunction(functionName);

    if (!CalleeF)
        return LogErrorV("Unknown method referenced: " + functionName);

    std::vector<std::shared_ptr<ASTNode>> arguments;
    arguments.push_back(std::make_shared<VariableAccessNode>(expressionToken(), false));
    for (auto &arg: m_arguments)
    {
        arguments.push_back(arg);
    }
    // If argument mismatch error.
    if (CalleeF->arg_size() != arguments.size() && !CalleeF->isVarArg())
    {
        std::cerr << "incorrect argument size for call " << functionName << " != " << CalleeF->arg_size() << "\n";
        return LogErrorV("Incorrect # arguments passed");
    }

    std::vector<llvm::Value *> ArgsV;
    for (unsigned argumentIndex = 0; argumentIndex < arguments.size(); ++argumentIndex)
    {

        std::optional<FunctionArgument> argType = functionDefinition->getParam(argumentIndex);

        if (argType.has_value())
            context->loadValue = !argType.value().isReference;

        auto argValue = arguments[argumentIndex]->codegen(context);
        context->loadValue = true;

        if (argType.has_value() && argType.value().isReference)
        {
            ArgsV.push_back(argValue);
        }
        else if (argType.has_value() && !argType.value().type->isSimpleType())
        {
            auto fieldName = functionDefinition->name() + "_" + argType->argumentName;
            const auto llvmArgType = argType->type->generateLlvmType(context);

            auto memcpyCall = llvm::Intrinsic::getDeclaration(
                    context->module().get(), llvm::Intrinsic::memcpy,
                    {context->builder()->getPtrTy(), context->builder()->getPtrTy(), context->builder()->getInt64Ty()});
            std::vector<llvm::Value *> memcpyArgs;
            llvm::AllocaInst *alloca = context->builder()->CreateAlloca(llvmArgType, nullptr, fieldName + "_ptr");

            const llvm::DataLayout &DL = context->module()->getDataLayout();
            uint64_t structSize = DL.getTypeAllocSize(argType->type->generateLlvmType(context));


            memcpyArgs.push_back(context->builder()->CreateBitCast(alloca, context->builder()->getPtrTy()));
            memcpyArgs.push_back(context->builder()->CreateBitCast(argValue, context->builder()->getPtrTy()));
            memcpyArgs.push_back(context->builder()->getInt64(structSize));
            memcpyArgs.push_back(context->builder()->getFalse());

            context->builder()->CreateCall(memcpyCall, memcpyArgs);

            ArgsV.push_back(alloca);
        }
        else
        {
            ArgsV.push_back(argValue);
        }


        if (!ArgsV.back())
            return nullptr;
    }

    llvm::AllocaInst *allocInst = nullptr;
    auto returnType = functionDefinition->returnType();
    if (returnType && returnType->baseType == VariableBaseType::String)
    {
        allocInst = context->builder()->CreateAlloca(returnType->generateLlvmType(context));
    }

    auto callInst = context->builder()->CreateCall(CalleeF, ArgsV);
    for (size_t i = 0, e = arguments.size(); i != e; ++i)
    {
        std::optional<FunctionArgument> argType = functionDefinition->getParam(static_cast<unsigned>(i));

        if (argType.has_value() && argType.value().type->baseType == VariableBaseType::Struct &&
            !argType.value().isReference)
        {
            auto llvmArgType = argType->type->generateLlvmType(context);

            callInst->addParamAttr(static_cast<unsigned>(i), llvm::Attribute::NoUndef);
            callInst->addParamAttr(static_cast<unsigned>(i),
                                   llvm::Attribute::getWithByValType(*context->context(), llvmArgType));
        }
    };

    if (allocInst)
    {
        context->builder()->CreateStore(callInst, allocInst);
        return allocInst;
    }
    return callInst;
}
std::string MethodCallNode::name() { return m_methodName.lexical(); }
std::string MethodCallNode::className() { return ASTNode::expressionToken().lexical(); }
std::shared_ptr<VariableType> MethodCallNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    return m_memberFunction.functionDefinition->returnType();
}
