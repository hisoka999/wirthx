#pragma once
#include "ASTNode.h"

class AddressNode : public ASTNode
{
private:
    std::string m_variableName;


public:
    explicit AddressNode(const Token &token);
    ~AddressNode() override = default;

    void print() override;

    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
    void typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
