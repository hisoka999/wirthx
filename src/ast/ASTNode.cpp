#include "ASTNode.h"

ASTNode::ASTNode(const Token &token) : m_token(token) {}

std::shared_ptr<VariableType> ASTNode::resolveType([[maybe_unused]] const std::unique_ptr<UnitNode> &unit,
                                                   ASTNode *parentNode)
{
    return std::make_shared<VariableType>();
}
