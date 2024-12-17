#include <cassert>
#include <iostream>
#include <llvm/IR/IRBuilder.h>

#include "ComparissionNode.h"
#include "compiler/Context.h"

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


llvm::Value *ComparrisionNode::codegen(std::unique_ptr<Context> &context)
{
    auto lhs = m_lhs->codegen(context);
    assert(lhs && "lhs of the comparison is null");
    auto rhs = m_rhs->codegen(context);
    assert(rhs && "rhs of the comparison is null");

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
            pred = llvm::CmpInst::ICMP_SLE;
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
