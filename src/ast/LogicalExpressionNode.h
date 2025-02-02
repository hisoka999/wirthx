#include "ASTNode.h"

enum class LogicalOperator : char
{
    AND,
    OR,
    NOT
};

class LogicalExpressionNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_lhs;
    std::shared_ptr<ASTNode> m_rhs;
    LogicalOperator m_operator;

public:
    LogicalExpressionNode(const Token &token, LogicalOperator op, const std::shared_ptr<ASTNode> &lhs,
                          const std::shared_ptr<ASTNode> &rhs);
    LogicalExpressionNode(const Token &token, LogicalOperator op, const std::shared_ptr<ASTNode> &rhs);
    ~LogicalExpressionNode() override = default;

    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;

    Token expressionToken() override;
};
