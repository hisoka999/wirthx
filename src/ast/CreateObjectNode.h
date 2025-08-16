//
// Created by stefan on 26.07.25.
//

#ifndef WIRTHX_CREATEOBJECTNODE_H
#define WIRTHX_CREATEOBJECTNODE_H
#include "ASTNode.h"
#include "types/ClassType.h"


class CreateObjectNode final : public ASTNode
{
private:
    std::shared_ptr<ClassType> m_classType;
    Token m_field;
    std::shared_ptr<FunctionDefinitionNode> m_memberFunction;
    bool m_inherited; // If true, the object is created in the parent class
    std::vector<std::shared_ptr<ASTNode>> m_arguments;

public:
    explicit CreateObjectNode(const Token &token, const std::shared_ptr<ClassType> &classType, Token field,
                              const std::shared_ptr<FunctionDefinitionNode> &memberFunction, bool inherited,
                              std::vector<std::shared_ptr<ASTNode>> arguments);
    void print() override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};


#endif // WIRTHX_CREATEOBJECTNODE_H
