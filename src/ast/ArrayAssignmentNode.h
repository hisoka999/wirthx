#pragma once
#include <string>
#include "ASTNode.h"


class ArrayAssignmentNode : public ASTNode
{
private:
    TokenWithFile m_arrayToken;
    std::string m_variableName;
    std::shared_ptr<ASTNode> m_indexNode;
    std::shared_ptr<ASTNode> m_expression;

public:
    ArrayAssignmentNode(TokenWithFile &arrayToken, const std::shared_ptr<ASTNode> &indexNode,
                        const std::shared_ptr<ASTNode> &expression);
    ~ArrayAssignmentNode() override = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
