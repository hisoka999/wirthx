#include "ASTNode.h"
#include "NumberNode.h"

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
    LogicalExpressionNode(LogicalOperator op, const std::shared_ptr<ASTNode> &lhs,
                          const std::shared_ptr<ASTNode> &rhs);
    LogicalExpressionNode(LogicalOperator op,
                          const std::shared_ptr<ASTNode> &rhs);
    ~LogicalExpressionNode() = default;

    void print() override;
    void eval(Stack &stack) override;
};