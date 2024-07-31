#include "FunctionCallNode.h"
#include <iostream>
#include "FunctionDefinitionNode.h"
#include "UnitNode.h"
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

FunctionCallNode::FunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args) :
    m_name(name), m_args(args)
{
}

void FunctionCallNode::print()
{
    std::cout << m_name << "(";
    for (auto &arg: m_args)
    {
        arg->print();
        std::cout << ",";
    }
    std::cout << ");\n";
}

void FunctionCallNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    context.parent = this;
    for (auto &arg: m_args)
    {
        arg->eval(context, outputStream);
    }
    auto &func = context.stack.getFunction(m_name);
    func->eval(context, outputStream);
    if (context.stack.has_var(m_name))
    {
        auto functionDefinition = context.unit->getFunctionDefinition(m_name);
        if (functionDefinition.value()->returnType()->baseType == VariableBaseType::Integer)
            context.stack.push_back(context.stack.get_var<int64_t>(m_name));
        else if (functionDefinition.value()->returnType()->baseType == VariableBaseType::String)
        {
            context.stack.push_back(context.stack.get_var<std::string_view>(m_name));
        }
    }
    context.parent = nullptr;
}

llvm::Value *FunctionCallNode::codegen(std::unique_ptr<Context> &context)
{
    // Look up the name in the global module table.
    std::string functionName = m_name;

    llvm::Function *CalleeF = context->TheModule->getFunction(functionName);

    if (!CalleeF && m_args.size() > 0)
    {
        ASTNode *parent = context.get()->ProgramUnit.get();
        if (context->TopLevelFunction)
        {
            auto def = context->ProgramUnit->getFunctionDefinition(std::string(context->TopLevelFunction->getName()));
            if (def)
            {
                parent = def.value().get();
            }
        }
        // look for alternative name
        auto arg1 = m_args.at(0)->resolveType(context->ProgramUnit, parent);
        switch (arg1->baseType)
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
        std::cerr << "incorrect argumentsize for call " << functionName << "(" << m_args.size()
                  << ") != " << CalleeF->arg_size() << "\n";
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

std::shared_ptr<VariableType> FunctionCallNode::resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    auto functionDefinition = unit->getFunctionDefinition(m_name);
    if (!functionDefinition)
    {
        return std::make_shared<VariableType>();
    }
    return functionDefinition.value()->returnType();
}


std::string FunctionCallNode::name() { return m_name; }
