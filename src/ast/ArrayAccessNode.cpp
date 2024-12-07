#include "ArrayAccessNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "exceptions/CompilerException.h"


ArrayAccessNode::ArrayAccessNode(const TokenWithFile arrayName, const std::shared_ptr<ASTNode> &indexNode) :
    m_arrayNameToken(arrayName), m_arrayName(std::string(arrayName.token.lexical)), m_indexNode(indexNode)
{
}

void ArrayAccessNode::print() {}


std::shared_ptr<VariableType> ArrayAccessNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    if (auto functionDefinition = dynamic_cast<FunctionDefinitionNode *>(parentNode))
    {
        auto param = functionDefinition->getParam(m_arrayName);
        if (param)
        {
            return param.value().type;
        }
    }
    auto definition = unit->getVariableDefinition(m_arrayName);
    if (definition)
    {
        auto array = std::dynamic_pointer_cast<ArrayType>(definition.value().variableType);

        return array->arrayBase;
    }
    return std::make_shared<VariableType>();
}

llvm::Value *ArrayAccessNode::codegen(std::unique_ptr<Context> &context)
{
    llvm::AllocaInst *V = context->NamedAllocations[m_arrayName];

    if (!V)
        return LogErrorV("Unknown variable name");

    auto arrayDef = context->ProgramUnit->getVariableDefinition(m_arrayName);
    if (!arrayDef)
    {
        return LogErrorV("Unknown variable name");
    }
    else
    {
        auto def = std::dynamic_pointer_cast<ArrayType>(arrayDef->variableType);
        auto index = m_indexNode->codegen(context);

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

            return context->Builder->CreateLoad(arrayBaseType, bounds);
        }

        if (llvm::isa<llvm::ConstantInt>(index))
        {
            llvm::ConstantInt *value = reinterpret_cast<llvm::ConstantInt *>(index);

            if (value->getSExtValue() < static_cast<int64_t>(def->low) ||
                value->getSExtValue() > static_cast<int64_t>(def->high))
            {
                throw CompilerException(ParserError{.file_name = m_arrayNameToken.fileName,
                                                    .token = m_arrayNameToken.token,
                                                    .message = "the array index is not in the defined range."});
            }
        }

        if (def->low > 0)
            index = context->Builder->CreateSub(
                    index, context->Builder->getIntN(index->getType()->getIntegerBitWidth(), def->low),
                    "array.index.sub");

        llvm::ArrayRef<llvm::Value *> idxList = {context->Builder->getInt64(0), index};

        auto arrayValue = context->Builder->CreateGEP(V->getAllocatedType(), V, idxList, "arrayindex", false);
        return context->Builder->CreateLoad(V->getAllocatedType()->getArrayElementType(), arrayValue);
    }
}
