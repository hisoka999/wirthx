#include "WhileNode.h"
#include <interpreter/Stack.h>

WhileNode::WhileNode(std::shared_ptr<ASTNode> loopCondition, std::vector<std::shared_ptr<ASTNode>> nodes) : m_loopCondition(loopCondition), m_nodes(nodes)
{
}

void WhileNode::print()
{
}

void WhileNode::eval(Stack &stack, std::ostream &outputStream)
{

    while (true)
    {
        m_loopCondition->eval(stack, outputStream);
        auto value = stack.pop_front();
        if (std::get<int64_t>(value) == 0)
            break;

        for (auto &node : m_nodes)
        {
            node->eval(stack, outputStream);
        }
    }
}

llvm::Value *WhileNode::codegen(std::unique_ptr<Context> &context)
{
    return nullptr;
}