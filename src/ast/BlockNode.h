#include "ASTNode.h"

class BlockNode : public ASTNode
{
private:
    const std::vector<std::shared_ptr<ASTNode>> m_expressions;

public:
    BlockNode(const std::vector<std::shared_ptr<ASTNode>> &expressions);
    ~BlockNode() = default;

    void print() override;
    void eval(Stack &stack) override;
};