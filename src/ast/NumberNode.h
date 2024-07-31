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
    void eval(InterpreterContext &context, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
    int64_t getValue();
};
