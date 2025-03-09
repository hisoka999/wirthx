#pragma once
#include "ASTNode.h"

class DoubleNode : public ASTNode
{
private:
    double m_value;

public:
    DoubleNode(const Token &token, double value);
    ~DoubleNode() override = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
    double getValue() const;
};
