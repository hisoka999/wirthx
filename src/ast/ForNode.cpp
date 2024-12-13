#include "ForNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"

ForNode::ForNode(std::string loopVariable, std::shared_ptr<ASTNode> &startExpression,
                 std::shared_ptr<ASTNode> &endExpression, std::vector<std::shared_ptr<ASTNode>> &body) :
    ASTNode(), m_loopVariable(loopVariable), m_startExpression(startExpression), m_endExpression(endExpression),
    m_body(body)
{
}

ForNode::~ForNode() {}

void ForNode::print() {}

llvm::Value *ForNode::codegen(std::unique_ptr<Context> &context)
{

    llvm::Value *startValue = m_startExpression->codegen(context);
    if (!startValue)
        return nullptr;

    auto &builder = context->Builder;
    auto &llvmContext = context->TheContext;
    // Make the new basic block for the loop header, inserting after current
    // block.
    // Within the loop, the variable is defined equal to the PHI node.  If it
    // shadows an existing variable, we have to restore it, so save it now.
    // auto *oldValue = context->NamedAllocations[m_loopVariable];
    // if (!oldValue)
    //     return nullptr;
    // builder->CreateStore(startValue, context->NamedAllocations[m_loopVariable]);

    llvm::Function *TheFunction = builder->GetInsertBlock()->getParent();
    llvm::BasicBlock *preheaderBB = builder->GetInsertBlock();
    llvm::BasicBlock *loopBB = llvm::BasicBlock::Create(*llvmContext, "for.body", TheFunction);
    llvm::BasicBlock *afterBB = llvm::BasicBlock::Create(*llvmContext, "for.cleanup", TheFunction);
    // Insert an explicit fall through from the current block to the LoopBB.
    builder->CreateBr(loopBB);

    // Start insertion in LoopBB.
    builder->SetInsertPoint(loopBB);

    // Start the PHI node with an entry for Start.
    // ASTNode *parent = context->ProgramUnit.get();
    // if (context->TopLevelFunction)
    // {
    //     auto def = context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str());
    //     if (def)
    //     {
    //         parent = def.value().get();
    //     }
    // }
    // auto endExpressionType = m_endExpression->resolveType(context->ProgramUnit, parent);
    unsigned int bitLength = 64;
    auto targetType = llvm::Type::getIntNTy(*llvmContext, bitLength);
    // if (auto integerType = std::dynamic_pointer_cast<IntegerType>(endExpressionType))
    // {
    //     bitLength = integerType->length;
    // }
    llvm::PHINode *Variable = builder->CreatePHI(targetType, 2, m_loopVariable);

    if (startValue->getType()->getIntegerBitWidth() != bitLength)
    {
        startValue = context->Builder->CreateIntCast(startValue, targetType, true, "startValue_cast");
    }
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
    llvm::Value *stepValue = builder->getIntN(bitLength, 1);

    llvm::Value *nextVar = builder->CreateAdd(Variable, stepValue, "nextvar");
    // builder->CreateStore(nextVar, context->NamedAllocations[m_loopVariable]);
    //  Compute the end condition.
    llvm::Value *EndCond = m_endExpression->codegen(context);
    if (!EndCond)
        return nullptr;
    if (EndCond->getType()->getIntegerBitWidth() != bitLength)
    {
        EndCond = context->Builder->CreateIntCast(EndCond, targetType, true, "lhs_cast");
    }

    // Convert condition to a bool by comparing non-equal to 0.0.
    EndCond = context->Builder->CreateCmp(llvm::CmpInst::ICMP_SLE, nextVar, EndCond, "for.loopcond");

    // Create the "after loop" block and insert it.
    llvm::BasicBlock *loopEndBB = builder->GetInsertBlock();

    // Insert the conditional branch into the end of loopEndBB.
    builder->CreateCondBr(EndCond, loopBB, afterBB);

    // Any new code will be inserted in AfterBB.
    builder->SetInsertPoint(afterBB);

    // Add a new entry to the PHI node for the backedge.
    Variable->addIncoming(nextVar, loopEndBB);

    // Restore the unshadowed variable.
    // context->NamedAllocations[m_loopVariable] = result;

    // for expr always returns 0.0.
    return llvm::Constant::getNullValue(llvm::Type::getInt64Ty(*llvmContext));
}
