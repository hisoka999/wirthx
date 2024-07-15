#include "FunctionCallNode.h"
#include "FunctionDefinitionNode.h"
#include "compiler/Context.h"
#include "interpreter/Stack.h"
#include <iostream>

FunctionCallNode::FunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args) : m_name(name), m_args(args)
{
}

void FunctionCallNode::print()
{
    std::cout << m_name << "(";
    for (auto &arg : m_args)
    {
        arg->print();
        std::cout << ",";
    }
    std::cout << ");\n";
}

void FunctionCallNode::eval(Stack &stack, std::ostream &outputStream)
{
    for (auto &arg : m_args)
    {
        arg->eval(stack, outputStream);
    }
    auto &func = stack.getFunction(m_name);
    func->eval(stack, outputStream);
    if (stack.has_var(m_name))
        stack.push_back(stack.get_var(m_name));
}

llvm::Value *FunctionCallNode::codegen(std::unique_ptr<Context> &context)
{
    // Look up the name in the global module table.
    std::string functionName = m_name;
    llvm::Function *CalleeF = context->TheModule->getFunction(functionName);

    if (!CalleeF && m_args.size() > 0)
    {
        // look for alternative name
        auto arg1 = m_args.at(0)->resolveType(context);
        switch (arg1.baseType)
        {
        case VariableBaseType::Integer:
            functionName += "_int";
            break;
        case VariableBaseType::String:
            functionName += "_str";
            break;
        default:
            break;
        }
        CalleeF = context->TheModule->getFunction(functionName);
    }

    if (!CalleeF)
        return LogErrorV("Unknown function referenced");

    // If argument mismatch error.
    if (CalleeF->arg_size() != m_args.size() && !CalleeF->isVarArg())
    {
        std::cerr << "incorrect argumentsize for call " << functionName << "(" << m_args.size() << ") != " << CalleeF->arg_size() << "\n";
        return LogErrorV("Incorrect # arguments passed");
    }
    std::vector<llvm::Value *> ArgsV;
    for (unsigned i = 0, e = m_args.size(); i != e; ++i)
    {
        ArgsV.push_back(m_args[i]->codegen(context));
        if (!ArgsV.back())
            return nullptr;
    }

    return context->Builder->CreateCall(CalleeF, ArgsV);
}