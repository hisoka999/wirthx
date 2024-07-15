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

struct Context;

class VariableType
{
public:
    VariableBaseType baseType = VariableBaseType::Unknown;
    std::string typeName = "";

    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context);
    static VariableType getInteger();
    static VariableType getString();
};
