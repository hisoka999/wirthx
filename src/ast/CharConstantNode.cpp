#include "CharConstantNode.h"
#include "compiler/Context.h"
#include "llvm/IR/Constants.h"

CharConstantNode::CharConstantNode(std::string_view literal) : m_literal(literal.at(0)) {}

void CharConstantNode::print() {}
llvm::Value *CharConstantNode::codegen(std::unique_ptr<Context> &context)
{
    return llvm::ConstantInt::get(*context->TheContext, llvm::APInt(8, m_literal));
}

std::shared_ptr<VariableType> CharConstantNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    return VariableType::getInteger(8);
}
