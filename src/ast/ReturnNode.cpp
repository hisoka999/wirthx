#include "ReturnNode.h"
#include <iostream>
#include "compiler/Context.h"

ReturnNode::ReturnNode(std::shared_ptr<ASTNode> expression) : m_expression(expression) {}

void ReturnNode::print()
{
    std::cout << "return ";
    m_expression->print();
    std::cout << "\n";
}


llvm::Value *ReturnNode::codegen(std::unique_ptr<Context> &context)
{
    auto RetVal = m_expression->codegen(context);
    context->Builder->CreateRet(RetVal);
    return nullptr;
}
