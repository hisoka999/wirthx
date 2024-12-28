#pragma once

#include "ASTNode.h"
#include "Token.h"

class ArrayAccessNode : public ASTNode
{
private:
    Token m_arrayNameToken;
    std::shared_ptr<ASTNode> m_indexNode;

public:
    ArrayAccessNode(Token arrayName, const std::shared_ptr<ASTNode> &indexNode);
    ~ArrayAccessNode() override = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
