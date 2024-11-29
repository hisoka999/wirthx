#include "NumberNode.h"
#include <iostream>
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

NumberNode::NumberNode(int64_t value, size_t numBits) : ASTNode(), m_value(value), m_numBits(numBits) {}

void NumberNode::print() { std::cout << m_value; }

void NumberNode::eval(InterpreterContext &context, [[maybe_unused]] std::ostream &outputStream)
{
    context.stack.push_back(m_value);
}

llvm::Value *NumberNode::codegen(std::unique_ptr<Context> &context)
{
    return llvm::ConstantInt::get(*context->TheContext, llvm::APInt(m_numBits, m_value));
}

std::shared_ptr<VariableType> NumberNode::resolveType([[maybe_unused]] const std::unique_ptr<UnitNode> &unit,
                                                      ASTNode *parentNode)
{
    return VariableType::getInteger(m_numBits);
}

int64_t NumberNode::getValue() { return m_value; }
