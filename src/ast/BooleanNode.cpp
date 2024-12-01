#include "BooleanNode.h"
#include <iostream>
#include "compiler/Context.h"


BooleanNode::BooleanNode(const bool value) : m_value(value) {}

void BooleanNode::print()
{
    if (m_value)
        std::cout << "true";
    else
        std::cout << "false";
}

llvm::Value *BooleanNode::codegen(std::unique_ptr<Context> &context)
{
    if (m_value)
        return context->Builder->getTrue();

    return context->Builder->getFalse();
}

std::shared_ptr<VariableType> BooleanNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    return VariableType::getBoolean();
}
