#pragma once

#include "ASTNode.h"

class ForNode : public ASTNode
{
private:
    std::string m_loopVariable;
    std::shared_ptr<ASTNode> m_startExpression;
    std::shared_ptr<ASTNode> m_endExpression;
    std::vector<std::shared_ptr<ASTNode>> m_body;

public:
    ForNode(std::string loopVariable, std::shared_ptr<ASTNode> &startExpression, std::shared_ptr<ASTNode> &endExpression, std::vector<std::shared_ptr<ASTNode>> &body);
    ~ForNode();

    void print() override;
    void eval(Stack &stack, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    VariableType resolveType(std::unique_ptr<Context> &context) override;
};
