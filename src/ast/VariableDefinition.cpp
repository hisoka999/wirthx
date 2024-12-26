#include "VariableDefinition.h"
#include <cassert>
#include <llvm/IR/IRBuilder.h>
#include <magic_enum/magic_enum.hpp>
#include "ASTNode.h"
#include "RecordType.h"
#include "compare.h"
#include "compiler/Context.h"
using namespace std::literals;

llvm::AllocaInst *VariableDefinition::generateCode(std::unique_ptr<Context> &context) const
{
    auto array = std::dynamic_pointer_cast<ArrayType>(this->variableType);
    if (array != nullptr)
    {
        auto arrayType = array->generateLlvmType(context);

        if (array->isDynArray)
        {
            return context->Builder->CreateAlloca(arrayType, nullptr, this->variableName);
        }
        return context->Builder->CreateAlloca(arrayType, nullptr, this->variableName);
    }


    switch (this->variableType->baseType)
    {
        case VariableBaseType::Integer:
        {
            auto type = this->variableType->generateLlvmType(context);
            auto allocation = context->Builder->CreateAlloca(type, nullptr, this->variableName);
            // if (!this->value)
            context->Builder->CreateStore(
                    context->Builder->getIntN(allocation->getAllocatedType()->getIntegerBitWidth(), 0), allocation);
            return allocation;
        }
        case VariableBaseType::Boolean:
        {
            auto allocation =
                    context->Builder->CreateAlloca(context->Builder->getInt1Ty(), nullptr, this->variableName);
            context->Builder->CreateStore(context->Builder->getFalse(), allocation);
            return allocation;
        }
        case VariableBaseType::Float:
        case VariableBaseType::Real:
            return context->Builder->CreateAlloca(llvm::Type::getDoubleTy(*context->TheContext), nullptr,
                                                  this->variableName);
        case VariableBaseType::Struct:
        {
            auto structType = std::dynamic_pointer_cast<RecordType>(this->variableType);
            if (structType != nullptr)
            {
                return context->Builder->CreateAlloca(structType->generateLlvmType(context), nullptr,
                                                      this->variableName);
            }
        }
        case VariableBaseType::String:
        {
            auto stringType = std::dynamic_pointer_cast<StringType>(this->variableType);
            if (stringType != nullptr)
            {
                return context->Builder->CreateAlloca(stringType->generateLlvmType(context), nullptr,
                                                      this->variableName);
            }
        }
        default:
            assert(false && "unsupported variable base type to generate variable definition");
            return nullptr;
    }
}


llvm::Value *VariableDefinition::generateCodeForConstant(std::unique_ptr<Context> &context) const
{
    llvm::Function *TheFunction = context->TopLevelFunction;
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());

    auto array = std::dynamic_pointer_cast<ArrayType>(this->variableType);

    return this->value->codegen(context);
}
