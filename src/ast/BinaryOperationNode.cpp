#include "BinaryOperationNode.h"
#include <iostream>
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"


BinaryOperationNode::BinaryOperationNode(Operator op, const std::shared_ptr<ASTNode> &lhs,
                                         const std::shared_ptr<ASTNode> &rhs) :
    ASTNode(), m_lhs(lhs), m_rhs(rhs), m_operator(op)
{
}

void BinaryOperationNode::print()
{
    m_lhs->print();
    std::cout << static_cast<char>(m_operator);
    m_rhs->print();
}

void BinaryOperationNode::eval(InterpreterContext &context, [[maybe_unused]] std::ostream &outputStream)
{
    m_lhs->eval(context, outputStream);
    m_rhs->eval(context, outputStream);
    auto lhs = context.stack.pop_front<int64_t>();
    auto rhs = context.stack.pop_front<int64_t>();
    switch (m_operator)
    {
        case Operator::PLUS:
            context.stack.push_back(lhs + rhs);
            break;
        case Operator::MINUS:
            context.stack.push_back(lhs - rhs);
            break;
        case Operator::MUL:
            context.stack.push_back(lhs * rhs);
            break;
    }
}

llvm::Value *BinaryOperationNode::codegen(std::unique_ptr<Context> &context)
{

    llvm::Value *lhs = m_lhs->codegen(context);
    llvm::Value *rhs = m_rhs->codegen(context);
    if (!lhs || !rhs)
        return nullptr;

    size_t maxBitWith = std::max(lhs->getType()->getIntegerBitWidth(), rhs->getType()->getIntegerBitWidth());
    auto targetType = llvm::IntegerType::get(*context->TheContext, maxBitWith);
    if (maxBitWith != lhs->getType()->getIntegerBitWidth())
    {
        lhs = context->Builder->CreateIntCast(lhs, targetType, true, "lhs_cast");
    }
    if (maxBitWith != rhs->getType()->getIntegerBitWidth())
    {
        rhs = context->Builder->CreateIntCast(rhs, targetType, true, "rhs_cast");
    }


    switch (m_operator)
    {
        case Operator::PLUS:
            return context->Builder->CreateAdd(lhs, rhs, "addtmp");
        case Operator::MINUS:
            return context->Builder->CreateSub(lhs, rhs, "subtmp");
        case Operator::MUL:
            return context->Builder->CreateMul(lhs, rhs, "multmp");
    }
    return nullptr;
}
