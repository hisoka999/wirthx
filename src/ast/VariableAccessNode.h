#pragma once
#include "ASTNode.h"
#include <string>

class VariableAccessNode : public ASTNode
{
private:
    std::string m_variableName;

public:
    VariableAccessNode(const std::string_view variableName);
    ~VariableAccessNode() = default;
    void print() override;
    void eval(Stack &stack) override;
};
