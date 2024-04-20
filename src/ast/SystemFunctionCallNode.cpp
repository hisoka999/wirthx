#include "SystemFunctionCallNode.h"
#include "../compare.h"
#include <vector>
#include "interpreter/Stack.h"
#include <iostream>

static std::vector<std::string> knownSystemCalls = {"writeln", "write"};

bool isKnownSystemCall(const std::string &name)
{
    for (auto &call : knownSystemCalls)
        if (iequals(call, name))
            return true;
    return false;
}

SystemFunctionCallNode::SystemFunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args)
    : FunctionCallNode(name, args)
{
}

void SystemFunctionCallNode::eval(Stack &stack)
{
    if (iequals(m_name, "writeln"))
    {
        for (auto &arg : m_args)
        {
            arg->eval(stack);
        }
        // todo get elements from stack
        auto value = stack.pop_front();
        if (std::holds_alternative<int64_t>(value))
        {
            std::cout << std::get<int64_t>(value) << "\n";
        }
        else
        {
            std::cout << std::get<std::string_view>(value) << "\n";
        }
    }
}