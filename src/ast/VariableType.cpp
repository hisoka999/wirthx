#include "ast/VariableType.h"
#include <cassert>
#include "compiler/Context.h"

VariableType::VariableType(VariableBaseType baseType, std::string typeName) : baseType(baseType), typeName(typeName) {}

llvm::Type *VariableType::generateLlvmType(std::unique_ptr<Context> &context)
{
    switch (this->baseType)
    {
        case VariableBaseType::Float:
        case VariableBaseType::Real:
            return llvm::Type::getDoubleTy(*context->TheContext);
        case VariableBaseType::String:
            return llvm::Type::getInt8PtrTy(*context->TheContext);
        default:
            assert(false && "unknown base type to generate llvm type for");
    }
}

std::shared_ptr<IntegerType> VariableType::getInteger(size_t length)
{
    auto integer = std::make_shared<IntegerType>();
    integer->baseType = VariableBaseType::Integer;
    integer->length = length;
    integer->typeName = "integer" + std::to_string(length);
    return integer;
}

std::shared_ptr<VariableType> VariableType::getString()
{
    return std::make_shared<VariableType>(VariableBaseType::String, "string");
}

std::shared_ptr<ArrayType> ArrayType::getArray(size_t low, size_t heigh, const std::shared_ptr<VariableType> &baseType)
{
    auto type = std::make_shared<ArrayType>();
    type->baseType = baseType->baseType;
    type->low = low;
    type->high = heigh;
    type->arrayBase = baseType;
    return type;
}


llvm::Type *IntegerType::generateLlvmType(std::unique_ptr<Context> &context)
{
    return llvm::IntegerType::get(*context->TheContext, this->length);
}
