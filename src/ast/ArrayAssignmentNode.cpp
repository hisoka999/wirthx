#include "ArrayAssignmentNode.h"

#include <utility>
#include "UnitNode.h"
#include "compiler/Context.h"
#include "exceptions/CompilerException.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "types/StringType.h"

ArrayAssignmentNode::ArrayAssignmentNode(const Token &arrayToken, const std::shared_ptr<ASTNode> &indexNode,
                                         const std::shared_ptr<ASTNode> &expression) :
    ASTNode(arrayToken), m_arrayToken(arrayToken), m_variableName(std::string(arrayToken.lexical())),
    m_indexNode(indexNode), m_expression(expression)
{
}

void ArrayAssignmentNode::print() {}


llvm::Value *ArrayAssignmentNode::codegen(std::unique_ptr<Context> &context)
{
    // Look this variable up in the function.
    llvm::Value *V = context->NamedAllocations[m_variableName];

    if (!V && context->TopLevelFunction)
    {
        for (auto &arg: context->TopLevelFunction->args())
        {
            if (arg.getName() == m_variableName)
            {
                V = context->TopLevelFunction->getArg(arg.getArgNo());
                break;
            }
        }
    }

    if (!V)
        return LogErrorV("Unknown variable name for array assignment: " + m_variableName);

    const auto result = m_expression->codegen(context);

    auto index = m_indexNode->codegen(context);
    auto arrayDef = context->ProgramUnit->getVariableDefinition(m_variableName);
    std::shared_ptr<VariableType> variableType = nullptr;

    if (context->TopLevelFunction)
    {
        if (auto functionDef = context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str()))
        {
            arrayDef = functionDef.value()->body()->getVariableDefinition(m_variableName);
            if (!arrayDef)
            {
                auto param = functionDef.value()->getParam(m_variableName);
                variableType = param.value().type;
            }
        }
    }
    if (arrayDef)
    {
        variableType = arrayDef.value().variableType;
    }
    if (!variableType)
    {
        return LogErrorV("Unknown variable name for array assignment: " + m_variableName);
    }
    if (const auto def = std::dynamic_pointer_cast<ArrayType>(variableType))
    {
        if (llvm::isa<llvm::ConstantInt>(index) && !def->isDynArray)
        {
            const auto value = reinterpret_cast<llvm::ConstantInt *>(index);

            if (value->getSExtValue() < static_cast<int64_t>(def->low) ||
                value->getSExtValue() > static_cast<int64_t>(def->high))
            {
                throw CompilerException(
                        ParserError{.token = m_arrayToken, .message = "the array index is not in the defined range."});
            }
        }

        if (def->low > 0)
            index = context->Builder->CreateSub(
                    index, context->Builder->getIntN(index->getType()->getIntegerBitWidth(), def->low), "subtmp");

        const auto llvmRecordType = def->generateLlvmType(context);

        if (def->isDynArray)
        {
            const auto arrayBaseType = def->arrayBase->generateLlvmType(context);

            const auto arrayPointerOffset = context->Builder->CreateStructGEP(llvmRecordType, V, 1, "array.ptr.offset");

            const auto loadResult = context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext),
                                                                 arrayPointerOffset);


            const auto bounds = context->Builder->CreateGEP(arrayBaseType, loadResult,
                                                            llvm::ArrayRef<llvm::Value *>{index}, "", true);

            context->Builder->CreateStore(result, bounds);
            return result;
        }

        const auto bounds = context->Builder->CreateGEP(llvmRecordType, V, {context->Builder->getInt64(0), index},
                                                        "arrayindex", false);

        context->Builder->CreateStore(result, bounds);
        return result;
    }
    if (const auto def = std::dynamic_pointer_cast<StringType>(variableType))
    {
        const auto llvmRecordType = def->generateLlvmType(context);
        const auto arrayBaseType = IntegerType::getInteger(8)->generateLlvmType(context);

        const auto arrayPointerOffset = context->Builder->CreateStructGEP(llvmRecordType, V, 2, "array.ptr.offset");

        const auto loadResult =
                context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), arrayPointerOffset);


        const auto bounds =
                context->Builder->CreateGEP(arrayBaseType, loadResult, llvm::ArrayRef<llvm::Value *>{index}, "", true);

        context->Builder->CreateStore(result, bounds);
    }
    return nullptr;
}
