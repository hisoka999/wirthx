#include "ASTNode.h"

ASTNode::ASTNode()
{
}

VariableType ASTNode::resolveType([[maybe_unused]] std::unique_ptr<Context> &context)
{
    return VariableType{};
}