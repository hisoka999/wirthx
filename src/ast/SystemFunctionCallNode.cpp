#include "SystemFunctionCallNode.h"
#include <iostream>
#include <vector>
#include "../compare.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

static std::vector<std::string> knownSystemCalls = {"writeln", "write", "printf", "exit"};

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
}

llvm::Value *SystemFunctionCallNode::codegen(std::unique_ptr<Context> &context)
{
    return FunctionCallNode::codegen(context);
}
