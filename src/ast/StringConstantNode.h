#pragma once

#include "ASTNode.h"
#include <string>

class StringConstantNode : public ASTNode
{
private:
    std::string_view m_literal;

public:
    StringConstantNode(std::string_view literal);
    ~StringConstantNode() = default;
    void print() override;
    void eval(Stack &stack) override;
};