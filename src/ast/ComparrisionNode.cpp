#include <cassert>
#include <iostream>
#include "ComparissionNode.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"
ComparrisionNode::ComparrisionNode(CMPOperator op, const std::shared_ptr<ASTNode> &lhs,
                                   const std::shared_ptr<ASTNode> &rhs) : m_lhs(lhs), m_rhs(rhs), m_operator(op)
{
}

void ComparrisionNode::print()
{
    m_lhs->print();
    switch (m_operator)
    {
        case CMPOperator::EQUALS:
            std::cout << "=";
            break;
        case CMPOperator::GREATER:
            std::cout << ">";
            break;
        case CMPOperator::GREATER_EQUAL:
            std::cout << ">=";
            break;
        case CMPOperator::LESS:
            std::cout << "<";
            break;
        case CMPOperator::LESS_EQUAL:
            std::cout << "<=";
            break;
        default:
            break;
    }
    m_rhs->print();
}

void compare_int(CMPOperator op, int64_t lhs, int64_t rhs, Stack &stack)
{
    switch (op)
    {
        case CMPOperator::EQUALS:
            stack.push_back(static_cast<int64_t>(lhs == rhs));
            break;
        case CMPOperator::GREATER:
            stack.push_back(static_cast<int64_t>(lhs > rhs));
            break;
        case CMPOperator::GREATER_EQUAL:
            stack.push_back(static_cast<int64_t>(lhs >= rhs));
            break;
        case CMPOperator::LESS:
            stack.push_back(static_cast<int64_t>(lhs < rhs));
            break;
        case CMPOperator::LESS_EQUAL:
            stack.push_back(static_cast<int64_t>(lhs <= rhs));
            break;
    }
}

void compare_str(CMPOperator op, std::string_view lhs, std::string_view rhs, Stack &stack)
{


    switch (op)
    {
        case CMPOperator::EQUALS:
            stack.push_back(static_cast<int64_t>(lhs == rhs));
            break;
        case CMPOperator::GREATER:
            stack.push_back(static_cast<int64_t>(lhs > rhs));
            break;
        case CMPOperator::GREATER_EQUAL:
            stack.push_back(static_cast<int64_t>(lhs >= rhs));
            break;
        case CMPOperator::LESS:
            stack.push_back(static_cast<int64_t>(lhs < rhs));
            break;
        case CMPOperator::LESS_EQUAL:
            stack.push_back(static_cast<int64_t>(lhs <= rhs));
            break;
    }
}
void ComparrisionNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    m_lhs->eval(context, outputStream);
    m_rhs->eval(context, outputStream);


    if (m_lhs->resolveType(context.unit, context.parent)->baseType == VariableBaseType::Integer)
    {
        auto lhs = context.stack.pop_front<int64_t>();
        auto rhs = context.stack.pop_front<int64_t>();
        compare_int(m_operator, lhs, rhs, context.stack);
    }
    else
    {
        auto lhs = context.stack.pop_front<std::string_view>();
        auto rhs = context.stack.pop_front<std::string_view>();
        compare_str(m_operator, lhs, rhs, context.stack);
    }
}
llvm::Value *ComparrisionNode::codegen(std::unique_ptr<Context> &context)
{
    auto lhs = m_lhs->codegen(context);
    assert(lhs && "lhs of the comparisson is null");
    auto rhs = m_rhs->codegen(context);
    assert(rhs && "rhs of the comparisson is null");

    llvm::CmpInst::Predicate pred = llvm::CmpInst::ICMP_EQ;
    switch (m_operator)
    {
        case CMPOperator::EQUALS:

            break;
        case CMPOperator::GREATER:
            pred = llvm::CmpInst::ICMP_SGT;
            break;
        case CMPOperator::GREATER_EQUAL:
            pred = llvm::CmpInst::ICMP_SGE;
            break;
        case CMPOperator::LESS:
            pred = llvm::CmpInst::ICMP_SLT;
            break;
        case CMPOperator::LESS_EQUAL:
            pred = llvm::CmpInst::ICMP_SLT;
            break;
        default:
            break;
    }
    if (lhs->getType() != rhs->getType())
    {
        if (lhs->getType()->isIntegerTy() && rhs->getType()->isIntegerTy())
        {
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
        }
    }

    return context->Builder->CreateCmp(pred, lhs, rhs);
}
