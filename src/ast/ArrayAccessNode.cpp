#include "ArrayAccessNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

ArrayAccessNode::ArrayAccessNode(const std::string_view arrayName, const std::shared_ptr<ASTNode> &indexNode) :
    m_arrayName(arrayName), m_indexNode(indexNode)
{
}

void ArrayAccessNode::print() {}

void ArrayAccessNode::eval(InterpreterContext &context, std::ostream &outputStream)
{

    auto arrayDef = context.unit->getVariableDefinition(m_arrayName);

    if (!context.stack.has_var(m_arrayName))
    {
    }
    else
    {
        auto array = context.stack.get_var<PascalIntArray>(m_arrayName);
        m_indexNode->eval(context, outputStream);
        auto index = context.stack.pop_front<int64_t>();
        context.stack.push_back(array[index]);
    }
}

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
        return definition.value().variableType;
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
        if (def->low > 0)
            index = context->Builder->CreateSub(
                    index, context->Builder->getIntN(index->getType()->getIntegerBitWidth(), def->low), "subtmp");

        llvm::ArrayRef<llvm::Value *> idxList = {context->Builder->getInt64(0), index};

        auto arrayValue = context->Builder->CreateGEP(V->getAllocatedType(), V, idxList, "arrayindex", false);
        return context->Builder->CreateLoad(V->getAllocatedType()->getArrayElementType(), arrayValue);
    }
}
