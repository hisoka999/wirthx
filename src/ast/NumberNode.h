#pragma once
#include "ASTNode.h"

class NumberNode : public ASTNode
{
private:
    int64_t m_value;

public:
    NumberNode(int64_t value);
    ~NumberNode() {};
    void print() override;
    void eval(Stack &stack, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    VariableType resolveType(std::unique_ptr<Context> &context) override;
};
