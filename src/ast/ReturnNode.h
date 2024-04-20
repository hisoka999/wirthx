#pragma once

#include "ASTNode.h"
#include <string>
#include <vector>

class ReturnNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_expression;

public:
    ReturnNode(std::shared_ptr<ASTNode> expression);
    ~ReturnNode() = default;
    void print() override;
    void eval(Stack &stack) override;
};
