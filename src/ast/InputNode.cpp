#include "InputNode.h"
#include <iostream>
#include "interpreter/InterpreterContext.h"

InputNode::InputNode(std::shared_ptr<ASTNode> outputTextNode, const std::string_view variableName) :
    m_outputTextNode(outputTextNode), m_variableName(variableName)
{
}

void InputNode::print()
{
    std::cout << "input ";
    m_outputTextNode->print();
    std::cout << m_variableName << "\n";
}
void InputNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    m_outputTextNode->eval(context, outputStream);
    auto text = context.stack.pop_front<std::string_view>();
    outputStream << text;
    int64_t input;
    std::cin >> input;
    context.stack.set_var(m_variableName, input);
}

llvm::Value *InputNode::codegen([[maybe_unused]] std::unique_ptr<Context> &context) { return nullptr; }
