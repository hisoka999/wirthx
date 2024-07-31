#pragma once
#include <string>
#include <vector>
#include "ASTNode.h"

class IfConditionNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_conditionNode;
    std::vector<std::shared_ptr<ASTNode>> m_ifExpressions;
    std::vector<std::shared_ptr<ASTNode>> m_elseExpressions;

    llvm::Value *codegenIf(std::unique_ptr<Context> &context);
    llvm::Value *codegenIfElse(std::unique_ptr<Context> &context);

public:
    IfConditionNode(std::shared_ptr<ASTNode> conditionNode, std::vector<std::shared_ptr<ASTNode>> ifExpressions,
                    std::vector<std::shared_ptr<ASTNode>> elseExpressions);
    ~IfConditionNode() = default;
    void print() override;
    void eval(InterpreterContext &context, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
