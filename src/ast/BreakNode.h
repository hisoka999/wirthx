#pragma once

#include "ASTNode.h"

class BreakNode : public ASTNode
{
private:
    /* data */
public:
    BreakNode();
    ~BreakNode();

    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
