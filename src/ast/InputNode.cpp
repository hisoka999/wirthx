#include "InputNode.h"
#include "interpreter/Stack.h"
#include <iostream>

InputNode::InputNode(std::shared_ptr<ASTNode> outputTextNode, const std::string_view variableName) : m_outputTextNode(outputTextNode), m_variableName(variableName)
{
}

void InputNode::print()
{
    std::cout << "input ";
    m_outputTextNode->print();
    std::cout << m_variableName << "\n";
}
void InputNode::eval(Stack &stack, std::ostream &outputStream)
{
    m_outputTextNode->eval(stack, outputStream);
    auto text = stack.pop_front<std::string_view>();
    outputStream << text;
    int64_t input;
    std::cin >> input;
    stack.set_var(m_variableName, input);
}
