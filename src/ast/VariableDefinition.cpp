#include "VariableDefinition.h"
#include "compare.h"
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