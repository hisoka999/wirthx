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
    Boolean,
    Unknown
};

namespace llvm
{
    class Type;
    class Value;
    class AllocaInst;
} // namespace llvm

struct Context;

class IntegerType;

class VariableType
{
public:
    VariableType(VariableBaseType baseType = VariableBaseType::Unknown, std::string typeName = "");
    virtual ~VariableType() = default;
    VariableBaseType baseType = VariableBaseType::Unknown;
    std::string typeName = "";

    virtual llvm::Type *generateLlvmType(std::unique_ptr<Context> &context);
    static std::shared_ptr<IntegerType> getInteger(size_t length = 32);
    static std::shared_ptr<VariableType> getBoolean();
    static std::shared_ptr<VariableType> getString();
};

class ArrayType : public VariableType
{
public:
    size_t low;
    size_t high;

    std::shared_ptr<VariableType> arrayBase;

    static std::shared_ptr<ArrayType> getArray(size_t low, size_t heigh, const std::shared_ptr<VariableType> &baseType);
};


class IntegerType : public VariableType
{
public:
    size_t length;
    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context) override;
};
