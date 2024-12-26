#pragma once

#include <cstddef>
#include <string>
#include "ASTNode.h"
#include "Token.h"

class FieldAccessNode : public ASTNode
{
private:
    Token m_element;
    std::string m_elementName;
    Token m_field;
    std::string m_fieldName;

public:
    FieldAccessNode(const Token &element, const Token &field);
    ~FieldAccessNode() = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
