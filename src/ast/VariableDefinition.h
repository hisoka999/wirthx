#pragma once
#include <string>
#include "VariableType.h"

class ASTNode;

struct VariableDefinition
{
    std::shared_ptr<VariableType> variableType;
    std::string variableName;
    size_t scopeId;
    std::shared_ptr<ASTNode> value = nullptr;
    bool constant = false;

    llvm::AllocaInst *generateCode(std::unique_ptr<Context> &context) const;
    llvm::Value *generateCodeForConstant(std::unique_ptr<Context> &context) const;
};
