#pragma once
#include "ASTNode.h"
#include <string>
#include <vector>

class IfConditionNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_conditionNode;
    std::vector<std::shared_ptr<ASTNode>> m_ifExpressions;
    std::vector<std::shared_ptr<ASTNode>> m_elseExpressions;

public:
    IfConditionNode(std::shared_ptr<ASTNode> conditionNode, std::vector<std::shared_ptr<ASTNode>> ifExpressions, std::vector<std::shared_ptr<ASTNode>> elseExpressions);
    ~IfConditionNode() = default;
    void print() override;
    void eval(Stack &stack) override;
};
