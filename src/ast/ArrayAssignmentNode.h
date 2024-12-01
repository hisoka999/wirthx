#pragma once
#include <string>
#include "ASTNode.h"


class ArrayAssignmentNode : public ASTNode
{
private:
    std::string m_variableName;
    std::shared_ptr<ASTNode> m_indexNode;
    std::shared_ptr<ASTNode> m_expression;

public:
    ArrayAssignmentNode(const std::string variableNam, const std::shared_ptr<ASTNode> &indexNode,
                        const std::shared_ptr<ASTNode> &expression);
    ~ArrayAssignmentNode() = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
