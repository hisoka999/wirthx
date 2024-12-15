#include "ASTNode.h"
#include "NumberNode.h"

enum class Operator : char
{
    PLUS = '+',
    MINUS = '-',
    MUL = '*'
};

class BinaryOperationNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_lhs;
    std::shared_ptr<ASTNode> m_rhs;
    Operator m_operator;

    llvm::Value *generateForInteger(llvm::Value *lhs, llvm::Value *rhs, std::unique_ptr<Context> &context);
    llvm::Value *generateForString(llvm::Value *lhs, llvm::Value *rhs, std::unique_ptr<Context> &context);

public:
    BinaryOperationNode(Operator op, const std::shared_ptr<ASTNode> &lhs, const std::shared_ptr<ASTNode> &rhs);
    ~BinaryOperationNode() = default;

    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
