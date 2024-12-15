#include "StringConstantNode.h"
#include <iostream>
#include "compiler/Context.h"


StringConstantNode::StringConstantNode(std::string literal) : ASTNode(), m_literal(literal) {}

void StringConstantNode::print() { std::cout << "\'" << m_literal << "\'"; }

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
        else if (m_literal[i] == '\'')
        {
            result += m_literal[i];
            if (m_literal.size() - 1 > i + 1 && m_literal[i + 1] == '\'')
            {
                i++;
            }
        }
        else
        {
            result += m_literal[i];
        }
    }
    auto varType = VariableType::getString();
    auto llvmRecordType = varType->generateLlvmType(context);
    auto stringAlloc = context->Builder->CreateAlloca(llvmRecordType, nullptr, "string_constant");

    auto constant = context->Builder->CreateGlobalString(result, ".str");

    auto arrayRefCountOffset =
            context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 0, "string.refCount.offset");
    auto arraySizeOffset = context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 1, "string.size.offset");


    auto arrayPointerOffset = context->Builder->CreateStructGEP(llvmRecordType, stringAlloc, 2, "string.ptr.offset");
    // auto arrayPointer =
    //         context->Builder->CreateAlignedLoad(arrayBaseType, arrayPointerOffset, alignment, "array.ptr");
    auto newSize = context->Builder->getInt64(result.size());
    // change array size
    context->Builder->CreateStore(context->Builder->getInt64(1), arrayRefCountOffset);
    context->Builder->CreateStore(newSize, arraySizeOffset);
    context->Builder->CreateStore(constant, arrayPointerOffset);

    return stringAlloc;
}

std::shared_ptr<VariableType> StringConstantNode::resolveType([[maybe_unused]] const std::unique_ptr<UnitNode> &unit,
                                                              ASTNode *parentNode)
{
    return VariableType::getString();
}
