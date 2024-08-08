#include "ForNode.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

ForNode::ForNode(std::string loopVariable, std::shared_ptr<ASTNode> &startExpression,
                 std::shared_ptr<ASTNode> &endExpression, std::vector<std::shared_ptr<ASTNode>> &body) :
    ASTNode(), m_loopVariable(loopVariable), m_startExpression(startExpression), m_endExpression(endExpression),
    m_body(body)
{
}

ForNode::~ForNode() {}

void ForNode::print() {}

void ForNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    m_startExpression->eval(context, outputStream);
    auto start_value = context.stack.pop_front<int64_t>();
    m_endExpression->eval(context, outputStream);
    auto end_value = context.stack.pop_front<int64_t>();

    context.stack.set_var(m_loopVariable, start_value);

    for (int64_t i = start_value; i <= end_value; i++)
    {
        context.stack.set_var(m_loopVariable, i);

        for (auto &exp: m_body)
        {
            exp->eval(context, outputStream);
            if (context.stack.stopBreakIfActive())
                return;
        }
    }
}

llvm::Value *ForNode::codegen(std::unique_ptr<Context> &context)
{
    using namespace llvm;

    Value *startValue = m_startExpression->codegen(context);
    if (!startValue)
        return nullptr;

    auto &builder = context->Builder;
    auto &llvmContext = context->TheContext;
    // Make the new basic block for the loop header, inserting after current
    // block.
    // Within the loop, the variable is defined equal to the PHI node.  If it
    // shadows an existing variable, we have to restore it, so save it now.
    auto *oldValue = context->NamedAllocations[m_loopVariable];
    if (!oldValue)
        return nullptr;
    builder->CreateStore(startValue, context->NamedAllocations[m_loopVariable]);

    Function *TheFunction = builder->GetInsertBlock()->getParent();
    BasicBlock *preheaderBB = builder->GetInsertBlock();
    BasicBlock *loopBB = BasicBlock::Create(*llvmContext, "for.body", TheFunction);
    BasicBlock *afterBB = BasicBlock::Create(*llvmContext, "for.cleanup", TheFunction);
    // Insert an explicit fall through from the current block to the LoopBB.
    builder->CreateBr(loopBB);

    // Start insertion in LoopBB.
    builder->SetInsertPoint(loopBB);

    // Start the PHI node with an entry for Start.
    PHINode *Variable = builder->CreatePHI(Type::getInt64Ty(*llvmContext), 2, m_loopVariable);
    Variable->addIncoming(startValue, preheaderBB);

    context->NamedValues[m_loopVariable] = Variable;

    context->BreakBlock.Block = afterBB;
    context->BreakBlock.BlockUsed = false;

    // Emit the body of the loop.  This, like any other expr, can change the
    // current BB.  Note that we ignore the value computed by the body, but don't
    // allow an error.
    for (auto &exp: m_body)
    {
        builder->SetInsertPoint(loopBB);
        exp->codegen(context);
    }
    context->BreakBlock.Block = nullptr;
    // Emit the step value.
    Value *stepValue = builder->getInt64(1);

    Value *nextVar = builder->CreateAdd(Variable, stepValue, "nextvar");
    builder->CreateStore(nextVar, context->NamedAllocations[m_loopVariable]);
    // Compute the end condition.
    Value *EndCond = m_endExpression->codegen(context);
    if (!EndCond)
        return nullptr;

    // Convert condition to a bool by comparing non-equal to 0.0.
    EndCond = context->Builder->CreateCmp(CmpInst::ICMP_SLE, nextVar, EndCond, "for.loopcond");

    // Create the "after loop" block and insert it.
    BasicBlock *loopEndBB = builder->GetInsertBlock();

    // Insert the conditional branch into the end of loopEndBB.
    builder->CreateCondBr(EndCond, loopBB, afterBB);

    // Any new code will be inserted in AfterBB.
    builder->SetInsertPoint(afterBB);

    // Add a new entry to the PHI node for the backedge.
    Variable->addIncoming(nextVar, loopEndBB);

    // Restore the unshadowed variable.
    // context->NamedAllocations[m_loopVariable] = result;

    // for expr always returns 0.0.
    return Constant::getNullValue(Type::getInt64Ty(*llvmContext));
}
