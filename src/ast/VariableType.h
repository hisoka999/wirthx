#pragma once
#include <memory>
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

namespace llvm
{
    class Type;
    class Value;
    class AllocaInst;
}

class Context;

struct VariableType
{
    VariableBaseType baseType = VariableBaseType::Unknown;
    std::string typeName = "";

    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context);
};