#pragma once

#include "ASTNode.h"

class WhileNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_loopCondition;
    std::vector<std::shared_ptr<ASTNode>> m_nodes;

public:
    WhileNode(const Token &token, std::shared_ptr<ASTNode> loopCondition, std::vector<std::shared_ptr<ASTNode>> nodes);
    ~WhileNode() override = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    void typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
