#pragma once
#include <string>
#include "ASTNode.h"
#include "Token.h"

class FieldAssignmentNode : public ASTNode
{
private:
    const Token m_variable;
    const std::string m_variableName;
    const Token m_field;
    const std::string m_fieldName;
    std::shared_ptr<ASTNode> m_expression;

public:
    FieldAssignmentNode(const Token &variable, const Token &field, const std::shared_ptr<ASTNode> &expression);
    ~FieldAssignmentNode() = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
