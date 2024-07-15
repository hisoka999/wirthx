#include "FunctionDefinitionNode.h"
#include "compiler/Context.h"
#include "interpreter/Stack.h"
#include <iostream>

FunctionDefinitionNode::FunctionDefinitionNode(std::string name, std::vector<FunctionArgument> params, std::shared_ptr<BlockNode> body, bool isProcedure, VariableType returnType)
    : m_name(name), m_params(params), m_body(body), m_isProcedure(isProcedure), m_returnType(returnType)
{
}

void FunctionDefinitionNode::print()
{
    if (m_isProcedure)
        std::cout << "procedure " + m_name + "(";
    else
        std::cout << "function " + m_name + "(";
    for (size_t i = 0; i < m_params.size(); ++i)
    {
        auto &param = m_params[i];
        if (param.isReference)
        {
            std::cout << "var ";
        }
        std::cout << param.argumentName + " :" + param.type.typeName;

        if (i != m_params.size() - 1)
        {
            std::cout << ",";
        }
    }
    std::cout << ")";
    if (m_isProcedure)
    {
        std::cout << "\n";
    }
    else
    {
        std::cout << ": " << m_returnType.typeName << ";\n";
    }
    m_body->print();

    // std::cout << "end;\n";
}

void FunctionDefinitionNode::eval(Stack &stack, std::ostream &outputStream)
{
    for (auto param : m_params)
    {
        auto value = stack.pop_front();
        stack.set_var(param.argumentName, value);
    }
    m_body->eval(stack, outputStream);
}

std::string &FunctionDefinitionNode::name()
{
    return m_name;
}

llvm::Value *FunctionDefinitionNode::codegen(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;
    for (auto &param : m_params)
    {
        params.push_back(param.type.generateLlvmType(context));
    }
    llvm::Type *resultType;
    if (m_isProcedure)
    {
        resultType = llvm::Type::getVoidTy(*context->TheContext);
    }
    else
    {
        resultType = m_returnType.generateLlvmType(context);
    }
    llvm::FunctionType *FT =
        llvm::FunctionType::get(resultType, params, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, m_name, context->TheModule.get());

    // Set names for all arguments.
    unsigned idx = 0;
    for (auto &arg : F->args())
        arg.setName(m_params[idx++].argumentName);

    // Create a new basic block to start insertion into.

    context->TopLevelFunction = F;
    m_body->setBlockName(m_name + "_block");
    m_body->codegen(context);

    if (m_isProcedure)
    {
        context->Builder->CreateRetVoid();

        verifyFunction(*F);
        return F;
    }
    // Finish off the function.

    // Validate the generated code, checking for consistency.

    return F;
}