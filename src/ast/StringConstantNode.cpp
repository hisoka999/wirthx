#include "StringConstantNode.h"
#include <iostream>
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

StringConstantNode::StringConstantNode(std::string_view literal) : ASTNode(), m_literal(literal) {}

void StringConstantNode::print() { std::cout << "\'" << m_literal << "\'"; }

void StringConstantNode::eval(InterpreterContext &context, 
                              [[maybe_unused]] std::ostream &outputStream)
{
    context.stack.push_back(m_literal);
}

llvm::Value *StringConstantNode::codegen(std::unique_ptr<Context> &context)
{
    std::string result;

    bool isEscape = false;
    for (size_t i = 0; i < m_literal.size(); ++i)
    {
        if (m_literal[i] == '\\')
        {
            isEscape = true;
        }
        else if (isEscape)
        {
            switch (m_literal[i])
            {
                case 'n':
                    result += 10;
                    break;
                case 'r':
                    result += 13;
                    break;
            }
            isEscape = false;
        }
        else
        {
            result += m_literal[i];
        }
    }
    return context->Builder->CreateGlobalString(result);
}

std::shared_ptr<VariableType> StringConstantNode::resolveType([[maybe_unused]] const std::unique_ptr<UnitNode> &unit,
                                                              ASTNode *parentNode)
{
    return VariableType::getString();
}
