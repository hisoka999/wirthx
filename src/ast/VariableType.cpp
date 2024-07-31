#include "ast/VariableType.h"
#include "compiler/Context.h"

VariableType::VariableType(VariableBaseType baseType, std::string typeName)
    : baseType(baseType), typeName(typeName)
{
}

llvm::Type *VariableType::generateLlvmType(std::unique_ptr<Context> &context)
{
    switch (this->baseType)
    {
    case VariableBaseType::Integer:
        return llvm::Type::getInt64Ty(*context->TheContext);
    case VariableBaseType::Float:
    case VariableBaseType::Real:
        return llvm::Type::getDoubleTy(*context->TheContext);
    case VariableBaseType::String:
        return llvm::Type::getInt8PtrTy(*context->TheContext);
    default:
        return nullptr;
    }
}

std::shared_ptr<VariableType> VariableType::getInteger()
{
    return std::make_shared<VariableType>(VariableBaseType::Integer, "integer");
}

std::shared_ptr<VariableType> VariableType::getString()
{
    return std::make_shared<VariableType>(VariableBaseType::String, "string");
}

std::shared_ptr<ArrayType> ArrayType::getArray(size_t low, size_t heigh, VariableBaseType baseType)
{
    auto type = std::make_shared<ArrayType>();
    type->baseType = baseType;
    type->low = low;
    type->heigh = heigh;
    return type;
}