#include "ArrayAssignmentNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"


ArrayAssignmentNode::ArrayAssignmentNode(const std::string variableName, const std::shared_ptr<ASTNode> &indexNode,
                                         const std::shared_ptr<ASTNode> &expression) :
    m_variableName(variableName), m_indexNode(indexNode), m_expression(expression)
{
}

void ArrayAssignmentNode::print() {}


llvm::Value *ArrayAssignmentNode::codegen(std::unique_ptr<Context> &context)
{
    // Look this variable up in the function.
    llvm::AllocaInst *V = context->NamedAllocations[m_variableName];

    if (!V)
        return LogErrorV("Unknown variable name");

    auto result = m_expression->codegen(context);

    auto index = m_indexNode->codegen(context);

    auto arrayDef = context->ProgramUnit->getVariableDefinition(m_variableName);
    if (!arrayDef)
    {
        return LogErrorV("Unknown variable name");
    }
    else
    {
        auto def = std::dynamic_pointer_cast<ArrayType>(arrayDef->variableType);

        if (def->low > 0)
            index = context->Builder->CreateSub(
                    index, context->Builder->getIntN(index->getType()->getIntegerBitWidth(), def->low), "subtmp");

        llvm::ArrayRef<llvm::Value *> idxList = {context->Builder->getInt64(0), index};

        if (def->isDynArray)
        {
            auto llvmRecordType = def->generateLlvmType(context);
            auto arrayBaseType = def->arrayBase->generateLlvmType(context);

            auto arrayPointerOffset = context->Builder->CreateStructGEP(llvmRecordType, V, 1, "array.ptr.offset");
            // const llvm::DataLayout &DL = context->TheModule->getDataLayout();
            // auto alignment = DL.getPrefTypeAlign(ptrType);
            auto loadResult = context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext),
                                                           arrayPointerOffset);


            auto bounds = context->Builder->CreateGEP(arrayBaseType, loadResult, llvm::ArrayRef<llvm::Value *>{index},
                                                      "", true);

            context->Builder->CreateStore(result, bounds);
            return result;
        }

        auto bounds = context->Builder->CreateGEP(V->getAllocatedType(), V, idxList, "arrayindex", false);

        context->Builder->CreateStore(result, bounds);
        return result;
    }
}
