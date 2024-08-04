#pragma once
#include <cstddef>
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
} // namespace llvm

struct Context;

class VariableType
{
public:
    VariableType(VariableBaseType baseType = VariableBaseType::Unknown, std::string typeName = "");
    virtual ~VariableType() = default;
    VariableBaseType baseType = VariableBaseType::Unknown;
    std::string typeName = "";

    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context);
    static std::shared_ptr<VariableType> getInteger();
    static std::shared_ptr<VariableType> getString();
};

class ArrayType : public VariableType
{
public:
    size_t low;
    size_t high;

    static std::shared_ptr<ArrayType> getArray(size_t low, size_t heigh, VariableBaseType baseType);
};
