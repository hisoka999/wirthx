#pragma once
#include <cstddef>
#include <memory>
#include <string>
#include "Token.h"

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
    Pointer,
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

class StringType;

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
    static std::shared_ptr<StringType> getString();
    static std::shared_ptr<VariableType> getPointer();

    bool operator==(const VariableType &other) const;
};

class FieldAccessableType
{
public:
    virtual ~FieldAccessableType() = default;
    virtual llvm::Value *generateFieldAccess(TokenWithFile &token, llvm::Value *indexValue,
                                             std::unique_ptr<Context> &context) = 0;
};

class ArrayType : public VariableType, public FieldAccessableType
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
    llvm::Value *generateFieldAccess(TokenWithFile &token, llvm::Value *indexValue,
                                     std::unique_ptr<Context> &context) override;

    bool operator==(const ArrayType &other) const
    {
        return this->low == other.low && this->high == other.high && this->arrayBase == other.arrayBase &&
               this->isDynArray == other.isDynArray;
    }
};


class IntegerType : public VariableType
{
public:
    size_t length;
    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context) override;
};

class StringType : public VariableType, public FieldAccessableType
{
private:
    llvm::Type *llvmType = nullptr;

public:
    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context) override;
    llvm::Value *generateFieldAccess(TokenWithFile &token, llvm::Value *indexValue,
                                     std::unique_ptr<Context> &context) override;

    bool operator==(const StringType &) const { return true; }
};
