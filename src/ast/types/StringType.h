#pragma once
#include "VariableType.h"


class StringType : public VariableType, public FieldAccessableType
{
private:
    llvm::Type *llvmType = nullptr;

public:
    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context) override;
    llvm::Value *generateFieldAccess(Token &token, llvm::Value *indexValue, std::unique_ptr<Context> &context) override;
    static std::shared_ptr<StringType> getString();

    bool operator==(const StringType &) const { return true; }
};
