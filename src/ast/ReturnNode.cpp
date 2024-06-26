#include "ReturnNode.h"
#include "interpreter/Stack.h"
#include <iostream>

ReturnNode::ReturnNode(std::shared_ptr<ASTNode> expression) : m_expression(expression)
{
}

void ReturnNode::print()
{
    std::cout << "return ";
    m_expression->print();
    std::cout << "\n";
}

void ReturnNode::eval(Stack &stack, std::ostream &outputStream)
{
    m_expression->eval(stack, outputStream);

    // TODO sowas wie call stacks existieren noch nicht
}