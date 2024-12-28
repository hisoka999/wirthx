#pragma once

#include "ASTNode.h"

class BreakNode : public ASTNode
{
private:
    /* data */
public:
    BreakNode(const Token &token);
    ~BreakNode() override = default;

    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
