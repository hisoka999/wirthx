#include "DoubleNode.h"

#include <llvm-18/llvm/IR/Constants.h>
#include <llvm-18/llvm/IR/IRBuilder.h>

#include "compiler/Context.h"
DoubleNode::DoubleNode(const Token &token, const double value) : ASTNode(token), m_value(value) {}
void DoubleNode::print() {}
llvm::Value *DoubleNode::codegen(std::unique_ptr<Context> &context)
{
    return llvm::ConstantFP::get(context->Builder->getDoubleTy(), m_value);
}
std::shared_ptr<VariableType> DoubleNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    return VariableType::getDouble();
}
double DoubleNode::getValue() const { return m_value; }
