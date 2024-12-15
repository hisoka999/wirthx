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
    auto definition = unit->getVariableDefinition(m_arrayName);
    std::shared_ptr<VariableType> varType = nullptr;
    if (!definition)
    {
        if (auto functionDefinition = dynamic_cast<FunctionDefinitionNode *>(parentNode))
        {
            auto param = functionDefinition->getParam(m_arrayName);
            if (param)
            {
                varType = param.value().type;
            }
        }
    }
    else
    {
        varType = definition->variableType;
    }

    if (varType)
    {
        if (auto array = std::dynamic_pointer_cast<ArrayType>(varType))
            return array->arrayBase;
        else if (auto array = std::dynamic_pointer_cast<StringType>(varType))
            return IntegerType::getInteger(8);
    }
    return std::make_shared<VariableType>();
}

llvm::Value *ArrayAccessNode::codegen(std::unique_ptr<Context> &context)
{
    llvm::Value *V = context->NamedAllocations[m_arrayName];


    if (!V)
    {
        for (auto &arg: context->TopLevelFunction->args())
        {
            if (arg.getName() == m_arrayName)
            {
                V = context->TopLevelFunction->getArg(arg.getArgNo());
                break;
            }
        }
    }

    if (!V)
        return LogErrorV("Unknown variable for array access: " + m_arrayName);

    ASTNode *parent = context->ProgramUnit.get();
    if (context->TopLevelFunction)
    {
        auto def = context->ProgramUnit->getFunctionDefinition(context->TopLevelFunction->getName().str());
        if (def)
        {
            parent = def.value().get();
        }
    }

    auto arrayDef = context->ProgramUnit->getVariableDefinition(m_arrayName);
    std::shared_ptr<VariableType> arrayDefType = nullptr;
    if (arrayDef)
    {
        arrayDefType = arrayDef->variableType;
    }

    if (auto functionDefinition = dynamic_cast<FunctionDefinitionNode *>(parent))
    {
        auto param = functionDefinition->getParam(m_arrayName);
        if (param)
        {
            arrayDefType = param.value().type;
        }
    }

    if (!arrayDefType)
    {
        return LogErrorV("Unknown variable for array access: " + m_arrayName);
    }
    auto fieldAccessType = std::dynamic_pointer_cast<FieldAccessableType>(arrayDefType);
    if (fieldAccessType)
    {

        auto index = m_indexNode->codegen(context);
        return fieldAccessType->generateFieldAccess(m_arrayNameToken, index, context);
    }
    else
    {
        return LogErrorV("variable can not access elements by [] operator: " + m_arrayName);
    }
}
