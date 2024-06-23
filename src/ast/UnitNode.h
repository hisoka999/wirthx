#pragma once

#include "ASTNode.h"
#include "ast/BlockNode.h"
#include "ast/FunctionDefinitionNode.h"

enum class UnitType
{
    UNIT,
    PROGRAM
};

class UnitNode : public ASTNode
{
private:
    UnitType m_unitType;
    std::string m_unitName;
    std::vector<std::shared_ptr<FunctionDefinitionNode>> m_functionDefinitions;

    std::shared_ptr<BlockNode> m_blockNode;

public:
    UnitNode(UnitType unitType, const std::string unitName, std::vector<std::shared_ptr<FunctionDefinitionNode>> m_functionDefinitions, const std::shared_ptr<BlockNode> &m_blockNode);
    ~UnitNode() = default;

    void print() override;
    void eval(Stack &stack, std::ostream &outputStream) override;

    std::vector<std::shared_ptr<FunctionDefinitionNode>> getFunctionDefinitions();
    void addFunctionDefinition(const std::shared_ptr<FunctionDefinitionNode> &functionDefinition);
    std::string getUnitName();
};