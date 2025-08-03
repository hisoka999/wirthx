#pragma once

#include "ASTNode.h"
#include "types/ClassType.h"


class MethodCallNode : public ASTNode
{
private:
    Token m_methodName;
    MemberFunction m_memberFunction;
    std::vector<std::shared_ptr<ASTNode>> m_arguments;

public:
    explicit MethodCallNode(const Token &token, Token methodName, MemberFunction memberFunction,
                            std::vector<std::shared_ptr<ASTNode>> arguments);
    void print() override;
    std::string callSignature(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode);
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::string name();

    [[nodiscard]] std::string className();

    [[nodiscard]] MemberFunction memberFunction() const { return m_memberFunction; }

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
