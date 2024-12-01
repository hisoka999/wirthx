#pragma once

#include <string>
#include <vector>
#include "ASTNode.h"

class ReturnNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_expression;

public:
    ReturnNode(std::shared_ptr<ASTNode> expression);
    ~ReturnNode() = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
