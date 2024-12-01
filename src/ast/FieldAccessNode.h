#pragma once

#include <cstddef>
#include <string>
#include "ASTNode.h"
#include "Token.h"

class FieldAccessNode : public ASTNode
{
private:
    TokenWithFile m_element;
    std::string m_elementName;
    TokenWithFile m_field;
    std::string m_fieldName;

public:
    FieldAccessNode(const TokenWithFile element, const TokenWithFile field);
    ~FieldAccessNode() = default;
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
