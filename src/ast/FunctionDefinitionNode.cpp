#include "FunctionDefinitionNode.h"
#include <iostream>
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

FunctionDefinitionNode::FunctionDefinitionNode(std::string name, std::vector<FunctionArgument> params,
                                               std::shared_ptr<BlockNode> body, bool isProcedure,
                                               std::shared_ptr<VariableType> returnType) :
    m_name(name), m_params(params), m_body(body), m_isProcedure(isProcedure), m_returnType(returnType)
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
        std::cout << param.argumentName + " :" + param.type->typeName;

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
        std::cout << ": " << m_returnType->typeName << ";\n";
    }
    m_body->print();

    // std::cout << "end;\n";
}

void FunctionDefinitionNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    for (auto param: m_params)
    {
        if (param.type->baseType == VariableBaseType::Integer)
        {
            auto value = context.stack.pop_front<int64_t>();
            context.stack.set_var(param.argumentName, value);
        }
        else if (param.type->baseType == VariableBaseType::String)
        {
            auto value = context.stack.pop_front<std::string_view>();
            context.stack.set_var(param.argumentName, value);
        }
    }
    m_body->eval(context, outputStream);
}

std::string &FunctionDefinitionNode::name() { return m_name; }

std::shared_ptr<VariableType> FunctionDefinitionNode::returnType() { return m_returnType; }

llvm::Value *FunctionDefinitionNode::codegen(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;
    for (auto &param: m_params)
    {
        params.push_back(param.type->generateLlvmType(context));
    }
    llvm::Type *resultType;
    if (m_isProcedure)
    {
        resultType = llvm::Type::getVoidTy(*context->TheContext);
    }
    else
    {
        resultType = m_returnType->generateLlvmType(context);
    }
    llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, false);

    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, m_name, context->TheModule.get());

    // Set names for all arguments.
    unsigned idx = 0;
    for (auto &arg: F->args())
        arg.setName(m_params[idx++].argumentName);

    // Create a new basic block to start insertion into.

    context->TopLevelFunction = F;
    m_body->setBlockName(m_name + "_block");
    m_body->codegen(context);

    if (m_isProcedure)
    {
        context->Builder->CreateRetVoid();

        verifyFunction(*F);
        context->TheFPM->run(*F, *context->TheFAM);

        return F;
    }
    // Finish off the function.

    // Validate the generated code, checking for consistency.
    verifyFunction(*F);
    context->TheFPM->run(*F, *context->TheFAM);

    return F;
}

std::optional<FunctionArgument> FunctionDefinitionNode::getParam(const std::string &paramName)
{
    for (auto &param: m_params)
    {
        if (param.argumentName == paramName)
        {
            return param;
        }
    }
    return std::nullopt;
}

std::shared_ptr<BlockNode> FunctionDefinitionNode::body() { return m_body; }
