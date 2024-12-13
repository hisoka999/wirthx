#include "FunctionDefinitionNode.h"
#include <iostream>
#include "FieldAccessNode.h"
#include "FieldAssignmentNode.h"
#include "RecordType.h"
#include "compiler/Context.h"


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

std::string &FunctionDefinitionNode::name() { return m_name; }

std::shared_ptr<VariableType> FunctionDefinitionNode::returnType() { return m_returnType; }

llvm::Value *FunctionDefinitionNode::codegen(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;

    for (auto &param: m_params)
    {

        if (param.isReference || param.type->baseType == VariableBaseType::Struct)
        {

            auto ptr = llvm::PointerType::getUnqual(param.type->generateLlvmType(context));
            params.push_back(ptr);
        }
        else
        {
            params.push_back(param.type->generateLlvmType(context));
        }
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

    llvm::Function *functionDefintion =
            llvm::Function::Create(FT, llvm::Function::ExternalLinkage, functionSignature(), context->TheModule.get());
    functionDefintion->setDSOLocal(true);
    functionDefintion->addFnAttr(llvm::Attribute::MustProgress);
    llvm::AttrBuilder b(*context->TheContext);
    b.addAttribute("frame-pointer", "all");
    functionDefintion->addFnAttrs(b);

    // Set names for all arguments.
    unsigned idx = 0;
    for (auto &arg: functionDefintion->args())
    {
        auto param = m_params[idx];
        if (!param.isReference && param.type->baseType == VariableBaseType::Struct)
        {
            arg.addAttr(llvm::Attribute::getWithByValType(*context->TheContext, param.type->generateLlvmType(context)));
            arg.addAttr(llvm::Attribute::NoUndef);
        }


        arg.setName(param.argumentName);
        idx++;
    }
    context->FunctionDefinitions[functionSignature()] = functionDefintion;
    // Create a new basic block to start insertion into.

    context->TopLevelFunction = functionDefintion;
    m_body->setBlockName(m_name + "_block");
    if (!m_isProcedure)
    {
        m_body->addVariableDefinition(VariableDefinition{.variableType = m_returnType,
                                                         .variableName = m_name,
                                                         .scopeId = 0,
                                                         .value = nullptr,
                                                         .constant = false});
    }
    m_body->codegen(context);

    if (m_isProcedure)
    {
        context->Builder->CreateRetVoid();

        verifyFunction(*functionDefintion);
        if (context->compilerOptions.buildMode == BuildMode::Release)
        {
            context->TheFPM->run(*functionDefintion, *context->TheFAM);
        }

        return functionDefintion;
    }
    else
    {
        context->Builder->CreateRet(context->Builder->CreateLoad(context->NamedAllocations[m_name]->getAllocatedType(),
                                                                 context->NamedAllocations[m_name]));
    }
    // Finish off the function.

    // Validate the generated code, checking for consistency.
    verifyFunction(*functionDefintion);
    if (context->compilerOptions.buildMode == BuildMode::Release)
    {
        context->TheFPM->run(*functionDefintion, *context->TheFAM);
    }

    return functionDefintion;
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

std::optional<FunctionArgument> FunctionDefinitionNode::getParam(const size_t index)
{
    if (m_params.size() > index)
    {
        return m_params[index];
    }
    return std::nullopt;
}

std::shared_ptr<BlockNode> FunctionDefinitionNode::body() { return m_body; }


std::string FunctionDefinitionNode::functionSignature()
{
    auto result = m_name + "(";
    for (size_t i = 0; i < m_params.size(); ++i)
    {
        result += m_params[i].type->typeName + ((i < m_params.size() - 1) ? "," : "");
    }
    result += ")";
    return result;
}
