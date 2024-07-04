#pragma once
#include "ASTNode.h"
#include "ast/VariableDefinition.h"

class BlockNode : public ASTNode
{
private:
    const std::vector<std::shared_ptr<ASTNode>> m_expressions;
    const std::vector<VariableDefinition> m_variableDefinitions;
    std::string m_blockname = "entry";

public:
    BlockNode(const std::vector<VariableDefinition> variableDefinitions, const std::vector<std::shared_ptr<ASTNode>> &expressions);
    ~BlockNode() = default;

    void print() override;
    void eval(Stack &stack, std::ostream &outputStream) override;
    void setBlockName(const std::string &name);
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};