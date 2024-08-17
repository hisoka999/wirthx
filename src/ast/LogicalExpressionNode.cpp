#include "LogicalExpressionNode.h"
#include <iostream>
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

LogicalExpressionNode::LogicalExpressionNode(LogicalOperator op, const std::shared_ptr<ASTNode> &lhs,
                                             const std::shared_ptr<ASTNode> &rhs) :
    m_lhs(lhs), m_rhs(rhs), m_operator(op)
{
}

LogicalExpressionNode::LogicalExpressionNode(LogicalOperator op, const std::shared_ptr<ASTNode> &rhs) :
    m_lhs(nullptr), m_rhs(rhs), m_operator(op)
{
}

void LogicalExpressionNode::print()
{
    switch (m_operator)
    {
        case LogicalOperator::AND:
        {
            m_lhs->print();
            std::cout << " and ";
            m_rhs->print();
        }
        break;
        case LogicalOperator::OR:
        {
            m_lhs->print();
            std::cout << " or ";
            m_rhs->print();
        }
        break;
        case LogicalOperator::NOT:
        {

            std::cout << " not ";
            m_rhs->print();
        }
        break;
        default:
            break;
    }
}

void LogicalExpressionNode::eval(InterpreterContext &context, std::ostream &outputStream)
{

    switch (m_operator)
    {
        case LogicalOperator::AND:
        {
            m_lhs->eval(context, outputStream);
            auto lhs = context.stack.pop_front<int64_t>();
            m_rhs->eval(context, outputStream);
            auto rhs = context.stack.pop_front<int64_t>();

            context.stack.push_back(static_cast<int64_t>(lhs && rhs));
        }
        break;
        case LogicalOperator::OR:
        {
            m_lhs->eval(context, outputStream);
            auto lhs = context.stack.pop_front<int64_t>();
            m_rhs->eval(context, outputStream);
            auto rhs = context.stack.pop_front<int64_t>();

            context.stack.push_back(static_cast<int64_t>(lhs || rhs));
        }
        break;
        case LogicalOperator::NOT:
        {
            m_rhs->eval(context, outputStream);
            auto rhs = context.stack.pop_front<int64_t>();
            context.stack.push_back(static_cast<int64_t>(!rhs));
        }
        break;
        default:
            break;
    }
}

llvm::Value *LogicalExpressionNode::codegen(std::unique_ptr<Context> &context)
{

    switch (m_operator)
    {
        case LogicalOperator::AND:
        {
            llvm::ArrayRef<llvm::Value *> ops = {m_lhs->codegen(context), m_rhs->codegen(context)};
            return context->Builder->CreateAnd(ops);
        }
        break;
        case LogicalOperator::OR:
        {
            llvm::ArrayRef<llvm::Value *> ops = {m_lhs->codegen(context), m_rhs->codegen(context)};
            return context->Builder->CreateOr(ops);
        }
        break;
        case LogicalOperator::NOT:
        {
            return context->Builder->CreateNot(m_rhs->codegen(context));
        }
        break;
        default:
            break;
    }
    return nullptr;
}
