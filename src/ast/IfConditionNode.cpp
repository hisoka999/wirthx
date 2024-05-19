#include "IfConditionNode.h"
#include "interpreter/Stack.h"
#include <iostream>

IfConditionNode::IfConditionNode(std::shared_ptr<ASTNode> conditionNode, std::vector<std::shared_ptr<ASTNode>> ifExpressions, std::vector<std::shared_ptr<ASTNode>> elseExpressions)
    : m_conditionNode(conditionNode), m_ifExpressions(ifExpressions), m_elseExpressions(elseExpressions)
{
}

void IfConditionNode::print()
{
    std::cout << "if ";
    m_conditionNode->print();
    std::cout << " then\n";

    for (auto &exp : m_ifExpressions)
    {
        exp->print();
    }
    if (m_elseExpressions.size() > 0)
    {
        std::cout << "else\n";
        for (auto &exp : m_elseExpressions)
        {
            exp->print();
        }
        // std::cout << "end;\n";
    }
}

void IfConditionNode::eval(Stack &stack, std::ostream &outputStream)
{
    m_conditionNode->eval(stack, outputStream);
    // check result
    auto result = stack.pop_front<int64_t>();
    if (result)
    {
        for (auto &exp : m_ifExpressions)
        {
            exp->eval(stack, outputStream);
        }
    }
    else
    {
        for (auto &exp : m_elseExpressions)
        {
            exp->eval(stack, outputStream);
        }
    }
}