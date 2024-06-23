#include "ast/UnitNode.h"
#include <iostream>

UnitNode::UnitNode(UnitType unitType, const std::string unitName, std::vector<std::shared_ptr<FunctionDefinitionNode>> functionDefinitions, const std::shared_ptr<BlockNode> &blockNode)
    : m_unitType(unitType), m_unitName(unitName), m_functionDefinitions(functionDefinitions), m_blockNode(blockNode)
{
}

void UnitNode::print()
{
    if (m_unitType == UnitType::PROGRAM)
    {
        std::cout << "program ";
    }
    else
    {
        std::cout << "unit ";
    }
    std::cout << m_unitName << "\n";
    for (auto def : m_functionDefinitions)
    {
        def->print();
    }

    m_blockNode->print();
}

void UnitNode::eval(Stack &stack, std::ostream &outputStream)
{
    m_blockNode->eval(stack, outputStream);
}

std::vector<std::shared_ptr<FunctionDefinitionNode>> UnitNode::getFunctionDefinitions()
{
    return m_functionDefinitions;
}

void UnitNode::addFunctionDefinition(const std::shared_ptr<FunctionDefinitionNode> &functionDefinition)
{
    m_functionDefinitions.push_back(functionDefinition);
}

std::string UnitNode::getUnitName()
{
    return m_unitName;
}