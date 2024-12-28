#pragma once

#include "ASTNode.h"

class RepeatUntilNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_loopCondition;
    std::vector<std::shared_ptr<ASTNode>> m_nodes;

public:
    RepeatUntilNode(const Token &token, std::shared_ptr<ASTNode> loopCondition,
                    std::vector<std::shared_ptr<ASTNode>> nodes);
    ~RepeatUntilNode() override = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
