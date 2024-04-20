#include "BlockNode.h"
#include <iostream>

BlockNode::BlockNode(const std::vector<std::shared_ptr<ASTNode>> &expressions) : m_expressions(expressions)
{
}

void BlockNode::print()
{
    std::cout << "begin\n";
    for (auto exp : m_expressions)
    {
        exp->print();
    }

    std::cout << "end;\n";
}

void BlockNode::eval(Stack &stack)
{
    for (auto &exp : m_expressions)
    {
        exp->eval(stack);
    }
}