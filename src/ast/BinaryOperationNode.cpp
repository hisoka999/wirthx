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

    llvm::Value *L = m_lhs->codegen(context);
    llvm::Value *R = m_rhs->codegen(context);
    if (!L || !R)
        return nullptr;

    switch (m_operator)
    {
        case Operator::PLUS:
            return context->Builder->CreateAdd(L, R, "addtmp");
        case Operator::MINUS:
            return context->Builder->CreateSub(L, R, "subtmp");
        case Operator::MUL:
            return context->Builder->CreateMul(L, R, "multmp");
    }
    return nullptr;
}
