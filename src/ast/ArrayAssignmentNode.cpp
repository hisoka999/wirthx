#include "ArrayAssignmentNode.h"

#include <utility>
#include "UnitNode.h"
#include "compiler/Context.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

ArrayAssignmentNode::ArrayAssignmentNode(std::string variableName, const std::shared_ptr<ASTNode> &indexNode,
                                         const std::shared_ptr<ASTNode> &expression) :
    m_variableName(std::move(variableName)), m_indexNode(indexNode), m_expression(expression)
{
}

void ArrayAssignmentNode::print() {}


llvm::Value *ArrayAssignmentNode::codegen(std::unique_ptr<Context> &context)
{
    // Look this variable up in the function.
    llvm::AllocaInst *V = context->NamedAllocations[m_variableName];

    if (!V)
        return LogErrorV("Unknown variable name for array assignment: " + m_variableName);

    const auto result = m_expression->codegen(context);

    auto index = m_indexNode->codegen(context);

    if (const auto arrayDef = context->ProgramUnit->getVariableDefinition(m_variableName); !arrayDef)
    {
        return LogErrorV("Unknown variable name for array assignment: " + m_variableName);
    }
    else
    {
        const auto def = std::dynamic_pointer_cast<ArrayType>(arrayDef->variableType);

        if (def->low > 0)
            index = context->Builder->CreateSub(
                    index, context->Builder->getIntN(index->getType()->getIntegerBitWidth(), def->low), "subtmp");

        llvm::ArrayRef<llvm::Value *> idxList = {context->Builder->getInt64(0), index};

        if (def->isDynArray)
        {
            const auto llvmRecordType = def->generateLlvmType(context);
            const auto arrayBaseType = def->arrayBase->generateLlvmType(context);

            const auto arrayPointerOffset = context->Builder->CreateStructGEP(llvmRecordType, V, 1, "array.ptr.offset");

            const auto loadResult = context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext),
                                                                 arrayPointerOffset);


            const auto bounds = context->Builder->CreateGEP(arrayBaseType, loadResult,
                                                            llvm::ArrayRef<llvm::Value *>{index}, "", true);

            context->Builder->CreateStore(result, bounds);
            return result;
        }

        const auto bounds = context->Builder->CreateGEP(V->getAllocatedType(), V, idxList, "arrayindex", false);

        context->Builder->CreateStore(result, bounds);
        return result;
    }
}
