#include "SystemFunctionCallNode.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <llvm/TargetParser/Triple.h>
#include <utility>
#include <vector>

#include <cinttypes>
#include <compiler/codegen.h>

#include "../compare.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "types/StringType.h"


static std::vector<std::string> knownSystemCalls = {"writeln",   "write",  "printf", "exit", "low",  "high",
                                                    "setlength", "length", "pchar",  "new",  "halt", "assert"};

bool isKnownSystemCall(const std::string &name)
{
    for (auto &call: knownSystemCalls)
    {
        if (iequals(call, name))
            return true;
    }
    return false;
}

SystemFunctionCallNode::SystemFunctionCallNode(const Token &token, std::string name,
                                               const std::vector<std::shared_ptr<ASTNode>> &args) :
    FunctionCallNode(token, std::move(name), args)
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
        llvm::Value *allocCall =
                context->Builder->CreateMalloc(indexType, //
                                               arrayBaseType, // Type of elements
                                               allocSize, // Number of elements
                                               nullptr // Optional array size multiplier (nullptr for scalar allocation)
                );
        const llvm::DataLayout &DL = context->TheModule->getDataLayout();

        auto alignment = DL.getPrefTypeAlign(arrayBaseType);

        context->Builder->CreateMemSet(allocCall, //
                                       context->Builder->getInt8(0), // Number of elements
                                       allocSize, // Optional array size multiplier (nullptr for scalar
                                       alignment //
        );
        return context->Builder->CreateStore(allocCall, arrayPointerOffset);
    }
    if (arrayType->baseType == VariableBaseType::String)
    {
        auto realType = std::dynamic_pointer_cast<StringType>(arrayType);
        auto indexType = VariableType::getInteger(64)->generateLlvmType(context);
        auto value = array->codegen(context);
        // const llvm::DataLayout &DL = context->TheModule->getDataLayout();
        // auto alignment = DL.getPrefTypeAlign(indexType);
        auto arrayBaseType = IntegerType::getInteger(8)->generateLlvmType(context);
        auto llvmRecordType = realType->generateLlvmType(context);


        // auto ptrType = llvm::PointerType::getUnqual(arrayBaseType);

        auto arraySizeOffset = context->Builder->CreateStructGEP(llvmRecordType, value, 1, "array.size.offset");


        auto arrayPointerOffset = context->Builder->CreateStructGEP(llvmRecordType, value, 2, "array.ptr.offset");
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
        // auto allocCall = context->Builder->CreateCall(context->TheModule->getFunction("malloc"), allocSize);
        llvm::Value *allocCall =
                context->Builder->CreateMalloc(indexType, //
                                               arrayBaseType, // Type of elements
                                               allocSize, // Number of elements
                                               nullptr // Optional array size multiplier (nullptr for scalar allocation)
                );

        return context->Builder->CreateStore(allocCall, arrayPointerOffset);
    }

    return nullptr;
}
llvm::Value *SystemFunctionCallNode::codegen_length(std::unique_ptr<Context> &context, ASTNode *parent) const
{
    const auto paramType = m_args[0]->resolveType(context->ProgramUnit, parent);
    if (const auto stringType = std::dynamic_pointer_cast<StringType>(paramType))
    {

        // const llvm::DataLayout &DL = context->TheModule->getDataLayout();
        const auto value = m_args[0]->codegen(context);
        const auto llvmRecordType = stringType->generateLlvmType(context);

        const auto arraySizeOffset = context->Builder->CreateStructGEP(llvmRecordType, value, 1, "length");
        const auto indexType = VariableType::getInteger(64)->generateLlvmType(context);

        return context->Builder->CreateSub(context->Builder->CreateLoad(indexType, arraySizeOffset, "loaded.length"),
                                           context->Builder->getInt64(1));
    }
    if (const auto arrayType = std::dynamic_pointer_cast<ArrayType>(paramType))
    {
        if (arrayType->isDynArray)
        {
            const auto value = m_args[0]->codegen(context);
            const auto llvmRecordType = arrayType->generateLlvmType(context);

            const auto arraySizeOffset =
                    context->Builder->CreateStructGEP(llvmRecordType, value, 0, "array.size.offset");
            const auto indexType = VariableType::getInteger(64)->generateLlvmType(context);

            return context->Builder->CreateLoad(indexType, arraySizeOffset);
        }
        return context->Builder->getInt64(arrayType->high - arrayType->low);
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
            if (context->TargetTriple->getOS() == llvm::Triple::Win32)
            {
                if (integerType->length > 32)
                    ArgsV.push_back(context->Builder->CreateGlobalString("%lli", "format_int64"));
                else if (integerType->length == 8)
                    ArgsV.push_back(context->Builder->CreateGlobalString("%c", "format_char"));
                else
                    ArgsV.push_back(context->Builder->CreateGlobalString("%i", "format_int"));
            }
            else
            {
                if (integerType->length > 32)
                    ArgsV.push_back(context->Builder->CreateGlobalString("%ld", "format_int64"));
                else if (integerType->length == 8)
                    ArgsV.push_back(context->Builder->CreateGlobalString("%c", "format_char"));
                else
                    ArgsV.push_back(context->Builder->CreateGlobalString("%d", "format_int"));
            }

            ArgsV.push_back(argValue);
        }
        else if (auto stringType = std::dynamic_pointer_cast<StringType>(type))
        {
            ArgsV.push_back(context->Builder->CreateGlobalString("%s", "format_string"));

            const auto stringStructPtr = argValue;
            const auto arrayPointerOffset = context->Builder->CreateStructGEP(
                    type->generateLlvmType(context), stringStructPtr, 2, "write.string.offset");
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

    if (context->TargetTriple->getOS() == llvm::Triple::Win32)
    {
        ArgsV.push_back(context->Builder->CreateGlobalString("\r\n"));
    }
    else
    {
        ArgsV.push_back(context->Builder->CreateGlobalString("\n"));
    }
    context->Builder->CreateCall(CalleeF, ArgsV);

    return nullptr;
}
llvm::Value *SystemFunctionCallNode::codegen_assert(std::unique_ptr<Context> &context, ASTNode *parent)
{
    const auto callingFunctionName = parent->expressionToken().lexical().c_str();
    const auto expression = m_args[0];
    const auto condition = context->Builder->CreateNot(expression->codegen(context));

    if (context->TargetTriple->getOS() == llvm::Triple::Linux)
    {
        codegen::codegen_ifexpr(
                context, condition,
                [expression, callingFunctionName](std::unique_ptr<Context> &ctx)
                {
                    const auto assertCall = ctx->TheModule->getFunction("__assert_fail");
                    std::vector<llvm::Value *> ArgsV;
                    ArgsV.push_back(ctx->Builder->CreateGlobalString(expression->expressionToken().lexical()));
                    ArgsV.push_back(
                            ctx->Builder->CreateGlobalString(expression->expressionToken().sourceLocation.filename));
                    ArgsV.push_back(ctx->Builder->getInt32(expression->expressionToken().row));
                    ArgsV.push_back(ctx->Builder->CreateGlobalString(callingFunctionName));
                    ctx->Builder->CreateCall(assertCall, ArgsV);
                });
    }
    else if (context->TargetTriple->getOS() == llvm::Triple::Win32)
    {
        codegen::codegen_ifexpr(context, condition,
                                [expression](std::unique_ptr<Context> &ctx)
                                {
                                    const auto assertCall = ctx->TheModule->getFunction("DbgBreak");
                                    std::vector<llvm::Value *> ArgsV;
                                    ArgsV.push_back(
                                            ctx->Builder->CreateGlobalString(expression->expressionToken().lexical()));
                                    ctx->Builder->CreateCall(assertCall, ArgsV);
                                });
    }
    else
    {
        assert(false && "assert is not supported");
    }
    return nullptr;
}
llvm::Value *SystemFunctionCallNode::codegen_new(std::unique_ptr<Context> &context, ASTNode *parent) const
{
    m_args[0]->codegen(context);
    const auto type = m_args[0]->resolveType(context->ProgramUnit, parent);
    if (const auto ptrType = std::dynamic_pointer_cast<PointerType>(type))
        return context->Builder->CreateAlloca(ptrType->pointerBase->generateLlvmType(context));

    return LogErrorV("argument is not a pointer type");
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
                const auto value = m_args[0]->codegen(context);
                const auto llvmRecordType = arrayType->generateLlvmType(context);

                const auto arraySizeOffset =
                        context->Builder->CreateStructGEP(llvmRecordType, value, 0, "array.size.offset");
                const auto indexType = VariableType::getInteger(64)->generateLlvmType(context);

                const auto length = context->Builder->CreateLoad(indexType, arraySizeOffset);

                return context->Builder->CreateSub(length, context->Builder->getInt64(1));
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
    else if (iequals(m_name, "new"))
    {
        return codegen_new(context, parent);
    }
    else if (iequals(m_name, "halt"))
    {
        auto argValue = m_args[0]->codegen(context);
        llvm::Function *CalleeF = context->TheModule->getFunction("exit");

        return context->Builder->CreateCall(CalleeF, argValue);
    }
    else if (iequals(m_name, "assert"))
    {
        return codegen_assert(context, parent);
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
