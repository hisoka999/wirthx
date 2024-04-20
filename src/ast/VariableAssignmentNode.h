#pragma once
#include "ASTNode.h"
#include <string>

class VariableAssignmentNode : public ASTNode
{
private:
    std::string m_variableName;
    std::shared_ptr<ASTNode> m_expression;

public:
    VariableAssignmentNode(const std::string_view variableName, const std::shared_ptr<ASTNode> &expression);
    ~VariableAssignmentNode() = default;
    void print() override;
    void eval(Stack &stack) override;
};
