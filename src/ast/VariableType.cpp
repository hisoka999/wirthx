#include "ast/VariableType.h"
#include "compiler/Context.h"

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

VariableType VariableType::getInteger()
{
    return VariableType{.baseType = VariableBaseType::Integer, .typeName = "integer"};
}

VariableType VariableType::getString()
{
    return VariableType{.baseType = VariableBaseType::String, .typeName = "string"};
}