#include "ASTNode.h"
#include "NumberNode.h"

enum class CMPOperator : char
{
    NOT_EQUALS,
    EQUALS,
    GREATER,
    GREATER_EQUAL,
    LESS,
    LESS_EQUAL
};

class ComparrisionNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_lhs;
    std::shared_ptr<ASTNode> m_rhs;
    CMPOperator m_operator;

public:
    ComparrisionNode(CMPOperator op, const std::shared_ptr<ASTNode> &lhs, const std::shared_ptr<ASTNode> &rhs);
    ~ComparrisionNode() = default;

    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
