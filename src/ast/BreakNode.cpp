#include "BreakNode.h"
#include "compiler/Context.h"
#include "llvm/IR/IRBuilder.h"

BreakNode::BreakNode(const Token &token) : ASTNode(token) {}


void BreakNode::print() {}

llvm::Value *BreakNode::codegen(std::unique_ptr<Context> &context)
{

    context->Builder->CreateBr(context->BreakBlock.Block);
    context->BreakBlock.BlockUsed = true;
    return nullptr;
}
