#pragma once
#include <string>
#include "ASTNode.h"
#include "Token.h"

class FieldAssignmentNode : public ASTNode
{
private:
    const TokenWithFile m_variable;
    const std::string m_variableName;
    const TokenWithFile m_field;
    const std::string m_fieldName;
    std::shared_ptr<ASTNode> m_expression;

public:
    FieldAssignmentNode(const TokenWithFile variable, const TokenWithFile field,
                        const std::shared_ptr<ASTNode> &expression);
    ~FieldAssignmentNode() = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
