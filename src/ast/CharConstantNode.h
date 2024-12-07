#pragma once

#include <string>
#include "ASTNode.h"

class CharConstantNode : public ASTNode
{
private:
    char m_literal;

public:
    CharConstantNode(std::string_view literal);
    ~CharConstantNode() = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
