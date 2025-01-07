#pragma once

#include <vector>
#include "ASTNode.h"

class ForNode : public ASTNode
{
private:
    std::string m_loopVariable;
    std::shared_ptr<ASTNode> m_startExpression;
    std::shared_ptr<ASTNode> m_endExpression;
    std::vector<std::shared_ptr<ASTNode>> m_body;
    int m_increment;

public:
    ForNode(const Token &token, std::string loopVariable, const std::shared_ptr<ASTNode> &startExpression,
            const std::shared_ptr<ASTNode> &endExpression, const std::vector<std::shared_ptr<ASTNode>> &body,
            int increment);
    ~ForNode() override = default;

    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    std::optional<std::shared_ptr<ASTNode>> block() override;

    void typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
