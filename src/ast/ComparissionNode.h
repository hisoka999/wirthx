#pragma once
#include "ASTNode.h"

enum class CMPOperator : char
{
    NOT_EQUALS,
    EQUALS,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL
};

class ComparrisionNode final : public ASTNode
{
private:
    Token m_operatorToken;
    std::shared_ptr<ASTNode> m_lhs;
    std::shared_ptr<ASTNode> m_rhs;
    CMPOperator m_operator;

public:
    ComparrisionNode(const Token &operatorToken, CMPOperator op, const std::shared_ptr<ASTNode> &lhs,
                     const std::shared_ptr<ASTNode> &rhs);
    ~ComparrisionNode() override = default;

    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    void typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;

    Token expressionToken() override;
};
