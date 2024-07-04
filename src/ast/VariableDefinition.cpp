#include "VariableDefinition.h"
#include "compare.h"
#include "compiler/Context.h"
using namespace std::literals;

std::optional<VariableType> determinVariableTypeByName(const std::string &name)
{
    auto integer = "integer"s;
    auto string = "string"s;
    auto real = "real"s;
    if (iequals(name, integer))
    {
        return VariableType{.baseType = VariableBaseType::Integer, .typeName = integer};
    }
    else if (iequals(name, string))
    {
        return VariableType{.baseType = VariableBaseType::String, .typeName = string};
    }
    else if (iequals(name, real))
    {
        return VariableType{.baseType = VariableBaseType::Real, .typeName = real};
    }

    return std::nullopt;
}

llvm::AllocaInst *VariableDefinition::generateCode(std::unique_ptr<Context> &context) const
{
    llvm::Function *TheFunction = context->TopLevelFunction;
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                           TheFunction->getEntryBlock().begin());

    switch (this->variableType.baseType)
    {
    case VariableBaseType::Integer:
        return TmpB.CreateAlloca(llvm::Type::getInt64Ty(*context->TheContext), nullptr,
                                 this->variableName);
    case VariableBaseType::Float:
    case VariableBaseType::Real:
        return TmpB.CreateAlloca(llvm::Type::getDoubleTy(*context->TheContext), nullptr,
                                 this->variableName);
    default:
        return nullptr;
    }
}