#pragma once
#include "VariableType.h"
#include <optional>
#include <string>

struct VariableDefinition
{
    VariableType variableType;
    std::string variableName;
    size_t scopeId;
};

std::optional<VariableType> determinVariableTypeByName(const std::string &name);