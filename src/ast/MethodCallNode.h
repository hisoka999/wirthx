#pragma once

#include "ASTNode.h"
#include "types/ClassType.h"


class MethodCallNode : public ASTNode
{
private:
    std::shared_ptr<ClassType> m_classType;
    Token m_methodName;
    MemberFunction m_memberFunction;
    bool m_inherited;
    std::vector<std::shared_ptr<ASTNode>> m_arguments;

public:
    explicit MethodCallNode(const Token &token, std::shared_ptr<ClassType> classType, Token methodName,
                            MemberFunction memberFunction, const bool inherited,
                            std::vector<std::shared_ptr<ASTNode>> arguments);
    void print() override;
    std::string callSignature(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode);
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::string name() const;

    [[nodiscard]] std::string className() const;

    [[nodiscard]] MemberFunction memberFunction() const { return m_memberFunction; }

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
    void typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
