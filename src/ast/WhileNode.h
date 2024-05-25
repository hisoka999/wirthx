#pragma once

#include "ASTNode.h"

class WhileNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_loopCondition;
    std::vector<std::shared_ptr<ASTNode>> m_nodes;

public:
    WhileNode(std::shared_ptr<ASTNode> loopCondition, std::vector<std::shared_ptr<ASTNode>> nodes);
    ~WhileNode() = default;
    void print() override;
    void eval(Stack &stack, std::ostream &outputStream) override;
};