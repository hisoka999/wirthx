#pragma once
#include <string>
#include "ASTNode.h"

class VariableAssignmentNode : public ASTNode
{
private:
    TokenWithFile m_variable;
    std::string m_variableName;
    std::shared_ptr<ASTNode> m_expression;

public:
    VariableAssignmentNode(const TokenWithFile variableName, const std::shared_ptr<ASTNode> &expression);
    ~VariableAssignmentNode() override = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    void typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
