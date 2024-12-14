#include "FunctionCallNode.h"
#include <iostream>
#include "FunctionDefinitionNode.h"
#include "RecordType.h"
#include "UnitNode.h"
#include "compiler/Context.h"


FunctionCallNode::FunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args) :
    m_name(name), m_args(args)
{
}

void FunctionCallNode::print()
{
    std::cout << m_name << "(";
    for (auto &arg: m_args)
    {
        arg->print();
        std::cout << ",";
    }
    std::cout << ");\n";
}

std::string FunctionCallNode::callSignature(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    ASTNode *parent = unit.get();
    if (parentNode != nullptr)
    {
        parent = parentNode;
    }
    std::string result = m_name + "(";
    for (size_t i = 0; i < m_args.size(); ++i)
    {
        auto arg = m_args.at(i)->resolveType(unit, parent);

        result += arg->typeName + ((i < m_args.size() - 1) ? "," : "");
    }
    result += ")";
    return result;
}

llvm::Value *FunctionCallNode::codegen(std::unique_ptr<Context> &context)
{
    // Look up the name in the global module table.
    ASTNode *parent = context->ProgramUnit.get();
    if (context->TopLevelFunction)
    {
        auto def = context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str());
        if (def)
        {
            parent = def.value().get();
        }
    }

    std::string functionName = callSignature(context->ProgramUnit, parent);

    llvm::Function *CalleeF = context->TheModule->getFunction(functionName);
    auto functionDefinition = context->ProgramUnit->getFunctionDefinition(functionName);
    if (!CalleeF)
    {
        functionDefinition = context->ProgramUnit->getFunctionDefinition(m_name);
        if (functionDefinition)
            CalleeF = context->TheModule->getFunction(functionDefinition.value()->functionSignature());
        if (CalleeF)
        {
            functionName = m_name;
        }
    }


    if (!CalleeF)
        return LogErrorV("Unknown function referenced: " + functionName);

    // If argument mismatch error.
    if (CalleeF->arg_size() != m_args.size() && !CalleeF->isVarArg())
    {
        std::cerr << "incorrect argumentsize for call " << functionName << " != " << CalleeF->arg_size() << "\n";
        return LogErrorV("Incorrect # arguments passed");
    }
    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = m_args.size(); i != e; ++i)
    {
        auto argValue = m_args[i]->codegen(context);
        std::optional<FunctionArgument> argType = std::nullopt;
        if (functionDefinition.has_value())
        {
            argType = functionDefinition.value()->getParam(i);
        }
        if (argType.has_value() && argType.value().isReference)
        {
            if (!argValue->getType()->isPointerTy())
            {
                auto fieldName = functionDefinition.value()->name() + "_" + argType->argumentName;

                llvm::AllocaInst *alloca =
                        context->Builder->CreateAlloca(argValue->getType(), nullptr, fieldName + "_ptr");
                context->Builder->CreateStore(argValue, alloca);

                ArgsV.push_back(alloca);
            }

            else
            {
                ArgsV.push_back(argValue);
            }
        }
        else if (argType.has_value() && argType.value().type->baseType == VariableBaseType::Struct)
        {
            auto fieldName = functionDefinition.value()->name() + "_" + argType->argumentName;
            auto llvmArgType = argType->type->generateLlvmType(context);

            auto memcpyCall = llvm::Intrinsic::getDeclaration(
                    context->TheModule.get(), llvm::Intrinsic::memcpy,
                    {context->Builder->getPtrTy(), context->Builder->getPtrTy(), context->Builder->getInt64Ty()});
            std::vector<llvm::Value *> memcopyArgs;
            llvm::AllocaInst *alloca = context->Builder->CreateAlloca(llvmArgType, nullptr, fieldName + "_ptr");

            const llvm::DataLayout &DL = context->TheModule->getDataLayout();
            uint64_t structSize = DL.getTypeAllocSize(argType->type->generateLlvmType(context));


            memcopyArgs.push_back(context->Builder->CreateBitCast(alloca, context->Builder->getPtrTy()));
            memcopyArgs.push_back(context->Builder->CreateBitCast(argValue, context->Builder->getPtrTy()));
            memcopyArgs.push_back(context->Builder->getInt64(structSize));
            memcopyArgs.push_back(context->Builder->getFalse());

            context->Builder->CreateCall(memcpyCall, memcopyArgs);

            ArgsV.push_back(alloca);
        }
        else
        {
            ArgsV.push_back(argValue);
        }


        if (!ArgsV.back())
            return nullptr;
    }

    auto callInst = context->Builder->CreateCall(CalleeF, ArgsV);
    for (unsigned i = 0, e = m_args.size(); i != e; ++i)
    {
        std::optional<FunctionArgument> argType = std::nullopt;
        if (functionDefinition.has_value())
        {
            argType = functionDefinition.value()->getParam(i);
        }
        if (argType.has_value() && argType.value().type->baseType == VariableBaseType::Struct &&
            !argType.value().isReference)
        {
            auto llvmArgType = argType->type->generateLlvmType(context);

            callInst->addParamAttr(i, llvm::Attribute::NoUndef);
            callInst->addParamAttr(i, llvm::Attribute::getWithByValType(*context->TheContext, llvmArgType));
        }
    };
    return callInst;
}

std::shared_ptr<VariableType> FunctionCallNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    auto functionDefinition = unit->getFunctionDefinition(callSignature(unit, parentNode));
    if (!functionDefinition)
    {
        functionDefinition = unit->getFunctionDefinition(m_name);
    }

    if (!functionDefinition)
    {
        return std::make_shared<VariableType>();
    }

    return functionDefinition.value()->returnType();
}


std::string FunctionCallNode::name() { return m_name; }
