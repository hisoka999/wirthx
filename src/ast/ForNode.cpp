#include "ForNode.h"
#include "compiler/Context.h"
#include "interpreter/Stack.h"

ForNode::ForNode(std::string loopVariable, std::shared_ptr<ASTNode> &startExpression, std::shared_ptr<ASTNode> &endExpression, std::vector<std::shared_ptr<ASTNode>> &body)
    : ASTNode(), m_loopVariable(loopVariable), m_startExpression(startExpression), m_endExpression(endExpression), m_body(body)
{
}

ForNode::~ForNode()
{
}

void ForNode::print()
{
}

void ForNode::eval(Stack &stack, std::ostream &outputStream)
{
    m_startExpression->eval(stack, outputStream);
    auto start_value = stack.pop_front<int64_t>();
    m_endExpression->eval(stack, outputStream);
    auto end_value = stack.pop_front<int64_t>();

    stack.set_var(m_loopVariable, start_value);

    for (int64_t i = start_value; i <= end_value; i++)
    {
        stack.set_var(m_loopVariable, i);

        for (auto &exp : m_body)
        {
            exp->eval(stack, outputStream);
            if (stack.stopBreakIfActive())
                return;
        }
    }
}

llvm::Value *ForNode::codegen(std::unique_ptr<Context> &context)
{
    using namespace llvm;

    Value *StartVal = m_startExpression->codegen(context);
    if (!StartVal)
        return nullptr;

    auto &Builder = context->Builder;
    auto &TheContext = context->TheContext;
    // Make the new basic block for the loop header, inserting after current
    // block.
    // Within the loop, the variable is defined equal to the PHI node.  If it
    // shadows an existing variable, we have to restore it, so save it now.
    auto *OldVal = context->NamedAllocations[m_loopVariable];
    if (!OldVal)
        return nullptr;
    Builder->CreateStore(StartVal, context->NamedAllocations[m_loopVariable]);

    Function *TheFunction = Builder->GetInsertBlock()->getParent();
    BasicBlock *PreheaderBB = Builder->GetInsertBlock();
    BasicBlock *LoopBB = BasicBlock::Create(*TheContext, "loop", TheFunction);
    BasicBlock *AfterBB =
        BasicBlock::Create(*TheContext, "afterloop", TheFunction);
    // Insert an explicit fall through from the current block to the LoopBB.
    Builder->CreateBr(LoopBB);

    // Start insertion in LoopBB.
    Builder->SetInsertPoint(LoopBB);

    // Start the PHI node with an entry for Start.
    PHINode *Variable =
        Builder->CreatePHI(Type::getInt64Ty(*TheContext), 2, m_loopVariable);
    Variable->addIncoming(StartVal, PreheaderBB);

    context->NamedValues[m_loopVariable] = Variable;

    context->BreakBlock = AfterBB;

    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB.  Note that we ignore the value computed by the body, but don't
    // allow an error.
    for (auto &exp : m_body)
    {
        Builder->SetInsertPoint(LoopBB);
        exp->codegen(context);
    }
    context->BreakBlock = nullptr;
    // Emit the step value.
    Value *StepVal = Builder->getInt64(1);

    Value *NextVar = Builder->CreateAdd(Variable, StepVal, "nextvar");
    Builder->CreateStore(NextVar, context->NamedAllocations[m_loopVariable]);
    // Compute the end condition.
    Value *EndCond = m_endExpression->codegen(context);
    if (!EndCond)
        return nullptr;

    // Convert condition to a bool by comparing non-equal to 0.0.
    EndCond = context->Builder->CreateCmp(CmpInst::ICMP_NE, NextVar, EndCond, "loopcond");

    // Create the "after loop" block and insert it.
    BasicBlock *LoopEndBB = Builder->GetInsertBlock();

    // Insert the conditional branch into the end of LoopEndBB.
    Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

    // Any new code will be inserted in AfterBB.
    Builder->SetInsertPoint(AfterBB);

    // Add a new entry to the PHI node for the backedge.
    Variable->addIncoming(NextVar, LoopEndBB);

    // Restore the unshadowed variable.
    // context->NamedAllocations[m_loopVariable] = result;

    // for expr always returns 0.0.
    return Constant::getNullValue(Type::getInt64Ty(*TheContext));
}

VariableType ForNode::resolveType(std::unique_ptr<Context> &context)
{
    return VariableType{};
}