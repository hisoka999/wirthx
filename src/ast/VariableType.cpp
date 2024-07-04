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
    default:
        return nullptr;
    }
}