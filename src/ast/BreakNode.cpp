#include "BreakNode.h"
#include "compiler/Context.h"
#include "interpreter/Stack.h"

BreakNode::BreakNode()
{
}

BreakNode::~BreakNode()
{
}

void BreakNode::print()
{
}

void BreakNode::eval(Stack &stack, std::ostream &outputStream)
{
    stack.startBreak();
}

llvm::Value *BreakNode::codegen(std::unique_ptr<Context> &context)
{

    context->Builder->CreateBr(context->BreakBlock);
    return nullptr;
}