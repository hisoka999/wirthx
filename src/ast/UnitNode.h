#pragma once

#include <map>
#include <set>
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
    std::map<std::string, std::shared_ptr<VariableType>> m_typeDefinitions;
    std::shared_ptr<BlockNode> m_blockNode;

public:
    UnitNode(UnitType unitType, const std::string unitName,
             std::vector<std::shared_ptr<FunctionDefinitionNode>> functionDefinitions,
             std::map<std::string, std::shared_ptr<VariableType>> typeDefinitions,
             const std::shared_ptr<BlockNode> &blockNode);
    ~UnitNode() = default;

    void print() override;

    std::vector<std::shared_ptr<FunctionDefinitionNode>> getFunctionDefinitions();
    std::optional<std::shared_ptr<FunctionDefinitionNode>> getFunctionDefinition(const std::string &functionName);
    void addFunctionDefinition(const std::shared_ptr<FunctionDefinitionNode> &functionDefinition);
    std::string getUnitName();
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    std::optional<VariableDefinition> getVariableDefinition(const std::string &name);
    std::set<std::string> collectLibsToLink();
};
