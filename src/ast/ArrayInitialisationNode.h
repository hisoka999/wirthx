#pragma once
#include <vector>


#include "ASTNode.h"
class ArrayInitialisationNode : public ASTNode
{
private:
    std::vector<std::shared_ptr<ASTNode>> m_arguments;

public:
    explicit ArrayInitialisationNode(const Token &token, const std::vector<std::shared_ptr<ASTNode>> &arguments);

    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
