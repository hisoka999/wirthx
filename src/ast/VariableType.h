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
    Array,
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
private:
    llvm::Type *llvmType = nullptr;

public:
    size_t low;
    size_t high;
    bool isDynArray;


    std::shared_ptr<VariableType> arrayBase;

    static std::shared_ptr<ArrayType> getFixedArray(size_t low, size_t heigh,
                                                    const std::shared_ptr<VariableType> &baseType);
    static std::shared_ptr<ArrayType> getDynArray(const std::shared_ptr<VariableType> &baseType);

    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context) override;
};


class IntegerType : public VariableType
{
public:
    size_t length;
    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context) override;
};
