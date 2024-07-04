#pragma once
#include "VariableType.h"
#include <optional>
#include <string>

struct VariableDefinition
{
    VariableType variableType;
    std::string variableName;
    size_t scopeId;

    llvm::AllocaInst *generateCode(std::unique_ptr<Context> &context) const;
};

std::optional<VariableType> determinVariableTypeByName(const std::string &name);