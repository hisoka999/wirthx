#pragma once
#include <optional>
#include "ASTNode.h"
#include "ast/VariableDefinition.h"

class BooleanNode : public ASTNode
{
private:
    const bool m_value;

public:
    BooleanNode(const bool value);
    ~BooleanNode() = default;

    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
