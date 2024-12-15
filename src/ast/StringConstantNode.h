#pragma once

#include <string>
#include "ASTNode.h"

class StringConstantNode : public ASTNode
{
private:
    std::string m_literal;

public:
    StringConstantNode(std::string literal);
    ~StringConstantNode() = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
