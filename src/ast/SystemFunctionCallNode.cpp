#include "SystemFunctionCallNode.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <vector>

#include "../compare.h"
#include "UnitNode.h"
#include "compiler/Context.h"


static std::vector<std::string> knownSystemCalls = {"writeln", "write",     "printf", "exit", "low",
                                                    "high",    "setlength", "length", "pchar"};

bool isKnownSystemCall(const std::string &name)
{
    for (auto &call: knownSystemCalls)
    {
        if (iequals(call, name))
            return true;
    }
    return false;
}

SystemFunctionCallNode::SystemFunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args) :
    FunctionCallNode(name, args)
{
}

llvm::Value *SystemFunctionCallNode::codegen_setlength(std::unique_ptr<Context> &context, ASTNode *parent)
{
    assert(m_args.size() == 2 && "setlength needs 2 arguments");
    auto array = m_args[0];
    auto newSize = m_args[1]->codegen(context);
    if (!newSize->getType()->isIntegerTy())
    {
        return nullptr;
    }

    auto arrayType = array->resolveType(context->ProgramUnit, parent);
    if (arrayType->baseType == VariableBaseType::Array)
    {
        auto realType = std::dynamic_pointer_cast<ArrayType>(arrayType);
        auto indexType = VariableType::getInteger(64)->generateLlvmType(context);
        auto value = array->codegen(context);
        // const llvm::DataLayout &DL = context->TheModule->getDataLayout();
        // auto alignment = DL.getPrefTypeAlign(indexType);
        auto arrayBaseType = realType->arrayBase->generateLlvmType(context);
        auto llvmRecordType = realType->generateLlvmType(context);


        // auto ptrType = llvm::PointerType::getUnqual(arrayBaseType);

        auto arraySizeOffset = context->Builder->CreateStructGEP(llvmRecordType, value, 0, "array.size.offset");


        auto arrayPointerOffset = context->Builder->CreateStructGEP(llvmRecordType, value, 1, "array.ptr.offset");
        // auto arrayPointer =
        //         context->Builder->CreateAlignedLoad(arrayBaseType, arrayPointerOffset, alignment, "array.ptr");
        if (64 != newSize->getType()->getIntegerBitWidth())
        {
            newSize = context->Builder->CreateIntCast(newSize, indexType, true, "lhs_cast");
        }

        // change array size
        context->Builder->CreateStore(newSize, arraySizeOffset);

        // allocate memory for pointer

        auto allocSize = context->Builder->CreateMul(
                newSize, context->Builder->getInt64(arrayBaseType->getPrimitiveSizeInBits()));
        auto allocCall = context->Builder->CreateCall(context->TheModule->getFunction("malloc"), allocSize);

        return context->Builder->CreateStore(allocCall, arrayPointerOffset);
    }

    return nullptr;
}
llvm::Value *SystemFunctionCallNode::codegen_length(std::unique_ptr<Context> &context, ASTNode *parent)
{
    const auto paramType = m_args[0]->resolveType(context->ProgramUnit, parent);
    if (const auto arrayType = std::dynamic_pointer_cast<StringType>(paramType))
    {

        // const llvm::DataLayout &DL = context->TheModule->getDataLayout();
        auto value = m_args[0]->codegen(context);
        auto llvmRecordType = arrayType->generateLlvmType(context);

        auto arraySizeOffset = context->Builder->CreateStructGEP(llvmRecordType, value, 1, "length");
        auto indexType = VariableType::getInteger(64)->generateLlvmType(context);

        return context->Builder->CreateLoad(indexType, arraySizeOffset, "loaded.length");
    }
    return nullptr;
}
llvm::Value *SystemFunctionCallNode::codegen_write(std::unique_ptr<Context> &context, ASTNode *parent)
{
    llvm::Function *CalleeF = context->TheModule->getFunction("printf");
    if (!CalleeF)
        LogErrorV("Unknown function referenced");
    for (auto &arg: m_args)
    {
        auto type = arg->resolveType(context->ProgramUnit, parent);
        auto argValue = arg->codegen(context);

        std::vector<llvm::Value *> ArgsV;
        if (auto integerType = std::dynamic_pointer_cast<IntegerType>(type))
        {
            if (integerType->length > 32)
                ArgsV.push_back(context->Builder->CreateGlobalString("%ld", "format_int64"));
            else if (integerType->length == 8)
                ArgsV.push_back(context->Builder->CreateGlobalString("%c", "format_char"));
            else
                ArgsV.push_back(context->Builder->CreateGlobalString("%d", "format_int"));
            ArgsV.push_back(argValue);
        }
        else if (auto stringType = std::dynamic_pointer_cast<StringType>(type))
        {
            ArgsV.push_back(context->Builder->CreateGlobalString("%s", "format_string"));

            const auto stringStructPtr = argValue;
            const auto arrayPointerOffset = context->Builder->CreateStructGEP(type->generateLlvmType(context),
                                                                              stringStructPtr, 2, "string.ptr.offset");
            const auto value = context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext),
                                                            arrayPointerOffset);


            ArgsV.push_back(value);
        }


        context->Builder->CreateCall(CalleeF, ArgsV);
    }
    return nullptr;
}
llvm::Value *SystemFunctionCallNode::codegen_writeln(std::unique_ptr<Context> &context, ASTNode *parent)
{
    codegen_write(context, parent);
    llvm::Function *CalleeF = context->TheModule->getFunction("printf");
    if (!CalleeF)
        LogErrorV("Unknown function referenced");
    std::vector<llvm::Value *> ArgsV;

    ArgsV.push_back(context->Builder->CreateGlobalString("\n"));
    context->Builder->CreateCall(CalleeF, ArgsV);
    return nullptr;
}
llvm::Value *SystemFunctionCallNode::codegen(std::unique_ptr<Context> &context)
{
    ASTNode *parent = context->ProgramUnit.get();
    if (context->TopLevelFunction)
    {
        auto def = context->ProgramUnit->getFunctionDefinition(std::string(context->TopLevelFunction->getName()));
        if (def)
        {
            parent = def.value().get();
        }
    }

    if (iequals(m_name, "low"))
    {

        auto paramType = m_args[0]->resolveType(context->ProgramUnit, parent);
        if (auto arrayType = std::dynamic_pointer_cast<ArrayType>(paramType))
        {
            if (arrayType->isDynArray)
            {
                return context->Builder->getInt64(0);
            }
            return context->Builder->getInt64(arrayType->low);
        }
    }
    else if (iequals(m_name, "high"))
    {
        auto paramType = m_args[0]->resolveType(context->ProgramUnit, parent);
        if (auto arrayType = std::dynamic_pointer_cast<ArrayType>(paramType))
        {
            if (arrayType->isDynArray)
            {
                // const llvm::DataLayout &DL = context->TheModule->getDataLayout();
                auto value = m_args[0]->codegen(context);
                auto llvmRecordType = arrayType->generateLlvmType(context);

                auto arraySizeOffset = context->Builder->CreateStructGEP(llvmRecordType, value, 0, "array.size.offset");
                auto indexType = VariableType::getInteger(64)->generateLlvmType(context);

                return context->Builder->CreateLoad(indexType, arraySizeOffset);
            }
            return context->Builder->getInt64(arrayType->high);
        }
    }
    else if (iequals(m_name, "length"))
    {
        return codegen_length(context, parent);
    }
    else if (iequals(m_name, "setlength"))
    {
        return codegen_setlength(context, parent);
    }
    else if (iequals(m_name, "write"))
    {
        return codegen_write(context, parent);
    }
    else if (iequals(m_name, "writeln"))
    {

        return codegen_writeln(context, parent);
    }
    else if (iequals(m_name, "pchar"))
    {
        auto stringStructPtr = m_args[0]->codegen(context);
        auto type = m_args[0]->resolveType(context->ProgramUnit, parent);
        const auto arrayPointerOffset = context->Builder->CreateStructGEP(type->generateLlvmType(context),
                                                                          stringStructPtr, 2, "string.ptr.offset");
        return context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), arrayPointerOffset);
    }
    return FunctionCallNode::codegen(context);
}


std::shared_ptr<VariableType> SystemFunctionCallNode::resolveType(const std::unique_ptr<UnitNode> &unitNode,
                                                                  ASTNode *parentNode)
{
    if (iequals(m_name, "low"))
    {
        return IntegerType::getInteger(64);
    }
    if (iequals(m_name, "high"))
    {
        return IntegerType::getInteger(64);
    }
    if (iequals(m_name, "length"))
    {
        return IntegerType::getInteger(64);
    }
    if (iequals(m_name, "setlength"))
    {
        return nullptr;
    }
    if (iequals(m_name, "pchar"))
    {
        return PointerType::getPointerTo(IntegerType::getInteger(8));
    }

    return nullptr;
}
