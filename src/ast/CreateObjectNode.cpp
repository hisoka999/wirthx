#include "CreateObjectNode.h"
#include <cassert>
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Intrinsics.h>
#include <utility>
#include "FunctionCallNode.h"
#include "FunctionDefinitionNode.h"
#include "UnitNode.h"
#include "VariableAccessNode.h"
#include "compare.h"
#include "compiler/Context.h"
CreateObjectNode::CreateObjectNode(const Token &token, const std::shared_ptr<ClassType> &classType, Token field,
                                   const std::shared_ptr<FunctionDefinitionNode> &memberFunction, const bool inherited,
                                   std::vector<std::shared_ptr<ASTNode>> arguments) :
    ASTNode(token), m_classType(classType), m_field(std::move(field)), m_memberFunction(memberFunction),
    m_inherited(inherited), m_arguments(std::move(arguments))
{
}
void CreateObjectNode::print() {}
llvm::Value *CreateObjectNode::codegen(std::unique_ptr<Context> &context)
{

    std::string functionName = to_lower(m_classType->typeName) + "." + m_memberFunction->functionSignature();

    llvm::Function *CalleeF = context->module()->getFunction(functionName);

    std::vector<std::shared_ptr<ASTNode>> arguments;


    if (m_inherited)
    {
        Token selfToken("self");
        arguments.push_back(std::make_shared<VariableAccessNode>(selfToken, true));
    }
    else
    {
        Token selfToken("self" + m_classType->typeName);
        VariableDefinition selfVariable{.variableType = m_classType,
                                        .variableName = selfToken.lexical(),
                                        .token = selfToken,
                                        .scopeId = 0,
                                        .value = nullptr,
                                        .constant = false};
        context->setNamedAllocation(selfToken.lexical(), selfVariable.generateCode(context));
        arguments.push_back(std::make_shared<VariableAccessNode>(selfToken, false));
    }
    for (auto &arg: m_arguments)
    {
        arguments.push_back(arg);
    }

    if (!CalleeF)
        return LogErrorV("Unknown constructor referenced: " + functionName);

    // If argument mismatch error.
    if (CalleeF->arg_size() != arguments.size() && !CalleeF->isVarArg())
    {
        std::cerr << "incorrect argument size for call " << functionName << " != " << CalleeF->arg_size() << "\n";
        return LogErrorV("Incorrect # arguments passed");
    }

    std::vector<llvm::Value *> ArgsV;
    for (unsigned argumentIndex = 0; argumentIndex < arguments.size(); ++argumentIndex)
    {

        std::optional<FunctionArgument> argType = m_memberFunction->getParam(argumentIndex);

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
            auto fieldName = m_memberFunction->name() + "_" + argType->argumentName;
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


    auto callInst = context->builder()->CreateCall(CalleeF, ArgsV);
    for (size_t i = 0, e = m_arguments.size(); i != e; ++i)
    {
        std::optional<FunctionArgument> argType = m_memberFunction->getParam(static_cast<unsigned>(i));

        if (argType.has_value() && argType.value().type->baseType == VariableBaseType::Struct &&
            !argType.value().isReference)
        {
            auto llvmArgType = argType->type->generateLlvmType(context);

            callInst->addParamAttr(static_cast<unsigned>(i), llvm::Attribute::NoUndef);
            callInst->addParamAttr(static_cast<unsigned>(i),
                                   llvm::Attribute::getWithByValType(*context->context(), llvmArgType));
        }
    };


    return context->namedAllocation("self" + m_classType->typeName);
}
std::shared_ptr<VariableType> CreateObjectNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    return m_classType;
}
void CreateObjectNode::typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    const auto functionDefinition = m_memberFunction;

    for (size_t i = 0; i < m_arguments.size(); ++i)
    {
        const auto arg = m_arguments[i];

        if (const auto paramType = functionDefinition->getParam(i); paramType.has_value())
        {
            arg->typeCheck(unit, parentNode);
            if (const auto argType = arg->resolveType(unit, parentNode); *argType != *(paramType.value().type))
            {
                throw std::runtime_error("Argument type mismatch for argument " + std::to_string(i) +
                                         " in function call " + functionDefinition->name());
            }
        }
    }
}
