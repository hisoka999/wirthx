#pragma once
#include "ASTNode.h"

class NumberNode : public ASTNode
{
private:
    int64_t m_value;
    size_t m_numBits;

public:
    NumberNode(int64_t value, size_t numBits);
    ~NumberNode() {};
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
    int64_t getValue();
};
