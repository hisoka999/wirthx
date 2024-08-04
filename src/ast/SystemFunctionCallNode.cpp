#include "SystemFunctionCallNode.h"
#include <iostream>
#include <vector>
#include "../compare.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

static std::vector<std::string> knownSystemCalls = {"writeln", "write", "printf", "exit", "low", "high"};

bool isKnownSystemCall(const std::string &name)
{
    for (auto &call: knownSystemCalls)
        if (iequals(call, name))
            return true;
    return false;
}

SystemFunctionCallNode::SystemFunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args) :
    FunctionCallNode(name, args)
{
}

void SystemFunctionCallNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    if (iequals(m_name, "writeln"))
    {
        VariableBaseType baseType;
        for (auto &arg: m_args)
        {
            arg->eval(context, outputStream);
            baseType = arg->resolveType(context.unit, context.parent)->baseType;
        }
        // todo get elements from stack

        if (baseType == VariableBaseType::Integer)
        {
            outputStream << context.stack.pop_front<int64_t>() << "\n";
        }
        else
        {
            outputStream << context.stack.pop_front<std::string_view>() << "\n";
        }
    }
    else if (iequals(m_name, "low"))
    {

        auto paramType = m_args[0]->resolveType(context.unit, context.parent);
        if (auto arrayType = std::dynamic_pointer_cast<ArrayType>(paramType))
        {
            context.stack.push_back(arrayType->low);
        }
    }
    else if (iequals(m_name, "high"))
    {
        auto paramType = m_args[0]->resolveType(context.unit, context.parent);
        if (auto arrayType = std::dynamic_pointer_cast<ArrayType>(paramType))
        {
            context.stack.push_back(arrayType->high);
        }
    }
}

llvm::Value *SystemFunctionCallNode::codegen(std::unique_ptr<Context> &context)
{
    ASTNode *parent = context->ProgramUnit.get();
    if (context->TopLevelFunction)
    {
        auto def = context->ProgramUnit->getFunctionDefinition(std::string(context->TopLevelFunction->getName()));
        if (def)
        {
            parent = def.value().get();
        }
    }

    if (iequals(m_name, "low"))
    {

        auto paramType = m_args[0]->resolveType(context->ProgramUnit, parent);
        if (auto arrayType = std::dynamic_pointer_cast<ArrayType>(paramType))
        {
            return context->Builder->getInt64(arrayType->low);
        }
    }
    else if (iequals(m_name, "high"))
    {
        auto paramType = m_args[0]->resolveType(context->ProgramUnit, parent);
        if (auto arrayType = std::dynamic_pointer_cast<ArrayType>(paramType))
        {
            return context->Builder->getInt64(arrayType->high);
        }
    }

    return FunctionCallNode::codegen(context);
}
