#include "StringConstantNode.h"
#include "compiler/Context.h"
#include "interpreter/Stack.h"
#include <iostream>

StringConstantNode::StringConstantNode(std::string_view literal) : ASTNode(), m_literal(literal)
{
}

void StringConstantNode::print()
{
    std::cout << "\'" << m_literal << "\'";
}

void StringConstantNode::eval(Stack &stack, [[maybe_unused]] std::ostream &outputStream)
{
    stack.push_back(m_literal);
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

    // auto arrayType = llvm::ArrayType::get(llvm::Type::getInt8Ty(*context->TheContext), m_literal.size());

    // return llvm::ConstantDataArray::getRaw(m_literal, m_literal.size(), arrayType);
    return context->Builder->CreateGlobalString(result);
}