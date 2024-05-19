#pragma once
#include <string>

enum class VariableBaseType
{
    Integer,
    Float,
    Real,
    String,
    Struct,
    Class,
    Unknown
};

struct VariableType
{
    VariableBaseType baseType = VariableBaseType::Unknown;
    std::string typeName = "";
};