#include "FunctionCallNode.h"
#include <iostream>
#include "FunctionDefinitionNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

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

void FunctionCallNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    context.parent = this;
    for (auto &arg: m_args)
    {
        arg->eval(context, outputStream);
    }
    auto &func = context.stack.getFunction(m_name);
    func->eval(context, outputStream);
    if (context.stack.has_var(m_name))
    {
        auto functionDefinition = context.unit->getFunctionDefinition(m_name);
        if (functionDefinition.value()->returnType()->baseType == VariableBaseType::Integer)
            context.stack.push_back(context.stack.get_var<int64_t>(m_name));
        else if (functionDefinition.value()->returnType()->baseType == VariableBaseType::String)
        {
            context.stack.push_back(context.stack.get_var<std::string_view>(m_name));
        }
    }
    context.parent = nullptr;
}

llvm::Value *FunctionCallNode::codegen(std::unique_ptr<Context> &context)
{
    // Look up the name in the global module table.
    std::string functionName = m_name;

    llvm::Function *CalleeF = context->TheModule->getFunction(functionName);
    auto functionDefinition = context->ProgramUnit->getFunctionDefinition(functionName);
    if (!CalleeF && m_args.size() > 0)
    {
        ASTNode *parent = context.get()->ProgramUnit.get();
        if (context->TopLevelFunction)
        {
            auto def = context->ProgramUnit->getFunctionDefinition(std::string(context->TopLevelFunction->getName()));
            if (def)
            {
                parent = def.value().get();
            }
        }
        // look for alternative name
        auto arg1 = m_args.at(0)->resolveType(context->ProgramUnit, parent);
        switch (arg1->baseType)
        {
            case VariableBaseType::Integer:
            {
                functionName += "_int";
                auto intType = std::dynamic_pointer_cast<IntegerType>(arg1);
                assert(intType && "variable base type is integer but it is not an IntegerType");
                functionName += std::to_string(intType->length);
                break;
            }
            case VariableBaseType::String:
                functionName += "_str";
                break;
            default:
                break;
        }
        CalleeF = context->TheModule->getFunction(functionName);
    }

    if (!CalleeF)
        return LogErrorV("Unknown function referenced");

    // If argument mismatch error.
    if (CalleeF->arg_size() != m_args.size() && !CalleeF->isVarArg())
    {
        std::cerr << "incorrect argumentsize for call " << functionName << "(" << m_args.size()
                  << ") != " << CalleeF->arg_size() << "\n";
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
            auto memcpyCall =
                    llvm::Intrinsic::getDeclaration(context->TheModule.get(), llvm::Intrinsic::memcpy,
                                                    {context->Builder->getInt8PtrTy(), context->Builder->getInt8PtrTy(),
                                                     context->Builder->getInt64Ty()});
            std::vector<llvm::Value *> memcopyArgs;
            llvm::AllocaInst *alloca = context->Builder->CreateAlloca(llvmArgType, nullptr, fieldName + "_ptr");

            const llvm::DataLayout &DL = context->TheModule->getDataLayout();
            uint64_t structSize = DL.getTypeAllocSize(argType->type->generateLlvmType(context));


            memcopyArgs.push_back(context->Builder->CreateBitCast(alloca, context->Builder->getInt8PtrTy()));
            memcopyArgs.push_back(context->Builder->CreateBitCast(argValue, context->Builder->getInt8PtrTy()));
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
    auto functionDefinition = unit->getFunctionDefinition(m_name);
    if (!functionDefinition)
    {
        return std::make_shared<VariableType>();
    }
    return functionDefinition.value()->returnType();
}


std::string FunctionCallNode::name() { return m_name; }
