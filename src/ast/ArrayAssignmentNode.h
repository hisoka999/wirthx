#pragma once
#include <string>
#include "ASTNode.h"


class ArrayAssignmentNode : public ASTNode
{
private:
    Token m_arrayToken;
    std::string m_variableName;
    std::shared_ptr<ASTNode> m_indexNode;
    std::shared_ptr<ASTNode> m_expression;
    void range_check(const std::shared_ptr<FieldAccessableType> &fieldAccesableType, std::unique_ptr<Context> &context);

public:
    ArrayAssignmentNode(const Token &arrayToken, const std::shared_ptr<ASTNode> &indexNode,
                        const std::shared_ptr<ASTNode> &expression);
    ~ArrayAssignmentNode() override = default;
    void print() override;

    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    Token expressionToken() override;
};
