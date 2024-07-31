#pragma once
#include "ASTNode.h"

class PrintNode : public ASTNode
{
private:
    std::vector<std::shared_ptr<ASTNode>> m_args;

public:
    PrintNode(std::vector<std::shared_ptr<ASTNode>> &args);
    ~PrintNode() = default;
    void print() override;
    void eval(InterpreterContext &context, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
