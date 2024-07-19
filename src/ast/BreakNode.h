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
    void eval(Stack &stack, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
