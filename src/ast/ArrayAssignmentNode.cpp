#include "ArrayAssignmentNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

ArrayAssignmentNode::ArrayAssignmentNode(const std::string variableName, const std::shared_ptr<ASTNode> &indexNode,
                                         const std::shared_ptr<ASTNode> &expression) :
    m_variableName(variableName), m_indexNode(indexNode), m_expression(expression)
{
}

void ArrayAssignmentNode::print() {}

void ArrayAssignmentNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    auto arrayDef = context.unit->getVariableDefinition(m_variableName);

    if (!context.stack.has_var(m_variableName))
    {
        auto type = std::dynamic_pointer_cast<ArrayType>(arrayDef.value().variableType);
        PascalIntArray array(type->low, type->high);
        m_indexNode->eval(context, outputStream);
        auto index = context.stack.pop_front<int64_t>();
        m_expression->eval(context, outputStream);
        array[index] = context.stack.pop_front<int64_t>();
        context.stack.set_var(m_variableName, array);
    }
    else
    {
        auto array = context.stack.get_var<PascalIntArray>(m_variableName);
        m_indexNode->eval(context, outputStream);

        auto index = context.stack.pop_front<int64_t>();
        m_expression->eval(context, outputStream);
        array[index] = context.stack.pop_front<int64_t>();
        context.stack.set_var(m_variableName, array);
    }
}

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

        auto bounds = context->Builder->CreateGEP(V->getAllocatedType(), V, idxList, "arrayindex", false);

        context->Builder->CreateStore(result, bounds);
        return result;
    }
}
