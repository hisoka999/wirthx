#pragma once
#include "ASTNode.h"
#include "ast/VariableDefinition.h"

class BlockNode : public ASTNode
{
private:
    const std::vector<std::shared_ptr<ASTNode>> m_expressions;
    const std::vector<VariableDefinition> m_variableDefinitions;

public:
    BlockNode(const std::vector<VariableDefinition> variableDefinitions, const std::vector<std::shared_ptr<ASTNode>> &expressions);
    ~BlockNode() = default;

    void print() override;
    void eval(Stack &stack, std::ostream &outputStream) override;
};