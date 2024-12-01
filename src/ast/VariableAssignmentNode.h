#pragma once
#include <string>
#include "ASTNode.h"

class VariableAssignmentNode : public ASTNode
{
private:
    std::string m_variableName;
    std::shared_ptr<ASTNode> m_expression;

public:
    VariableAssignmentNode(const std::string_view variableName, const std::shared_ptr<ASTNode> &expression);
    ~VariableAssignmentNode() = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
