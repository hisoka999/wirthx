#include "BreakNode.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

BreakNode::BreakNode() {}

BreakNode::~BreakNode() {}

void BreakNode::print() {}

void BreakNode::eval(InterpreterContext &context, std::ostream &outputStream) { context.stack.startBreak(); }

llvm::Value *BreakNode::codegen(std::unique_ptr<Context> &context)
{

    context->Builder->CreateBr(context->BreakBlock);
    return nullptr;
}
