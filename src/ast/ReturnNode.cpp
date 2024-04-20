#include "ReturnNode.h"
#include <iostream>
#include "interpreter/Stack.h"

ReturnNode::ReturnNode(std::shared_ptr<ASTNode> expression) : m_expression(expression)
{
}

void ReturnNode::print()
{
    std::cout << "return ";
    m_expression->print();
    std::cout << "\n";
}

void ReturnNode::eval(Stack &stack)
{
    m_expression->eval(stack);

    // TODO sowas wie call stacks existieren noch nicht
}