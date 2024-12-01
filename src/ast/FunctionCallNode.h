#pragma once

#include <string>
#include <vector>
#include "ASTNode.h"

class FunctionCallNode : public ASTNode
{
protected:
    std::string m_name;
    std::vector<std::shared_ptr<ASTNode>> m_args;

public:
    FunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args);
    ~FunctionCallNode() = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unitNode, ASTNode *parentNode) override;

    std::string name();
};
