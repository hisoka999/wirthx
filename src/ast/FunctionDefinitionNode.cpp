#include "FunctionDefinitionNode.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <utility>

#include "FieldAccessNode.h"
#include "FieldAssignmentNode.h"
#include "RecordType.h"
#include "compiler/Context.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"


FunctionDefinitionNode::FunctionDefinitionNode(std::string name, std::vector<FunctionArgument> params,
                                               std::shared_ptr<BlockNode> body, const bool isProcedure,
                                               std::shared_ptr<VariableType> returnType) :
    m_name(std::move(name)), m_externalName(m_name), m_params(std::move(params)), m_body(std::move(body)),
    m_isProcedure(isProcedure), m_returnType(std::move(returnType))
{
}

FunctionDefinitionNode::FunctionDefinitionNode(std::string name, std::string externalName, std::string libName,
                                               std::vector<FunctionArgument> params, const bool isProcedure,
                                               std::shared_ptr<VariableType> returnType) :
    m_name(std::move(name)), m_externalName(std::move(externalName)), m_libName(std::move(libName)),
    m_params(std::move(params)), m_body(nullptr), m_isProcedure(isProcedure), m_returnType(std::move(returnType))
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

        if (param.isReference || param.type->baseType == VariableBaseType::Struct ||
            param.type->baseType == VariableBaseType::String)
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

    llvm::Function *functionDefinition =
            llvm::Function::Create(FT, llvm::Function::ExternalLinkage, functionSignature(), context->TheModule.get());
    if (m_libName.empty())
    {
        functionDefinition->setDSOLocal(true);
        functionDefinition->addFnAttr(llvm::Attribute::MustProgress);
        llvm::AttrBuilder b(*context->TheContext);
        b.addAttribute("frame-pointer", "all");
        functionDefinition->addFnAttrs(b);
    }

    // Set names for all arguments.
    unsigned idx = 0;
    for (auto &arg: functionDefinition->args())
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
    context->FunctionDefinitions[functionSignature()] = functionDefinition;
    // Create a new basic block to start insertion into.

    context->TopLevelFunction = functionDefinition;
    if (m_body)
    {
        m_body->setBlockName(m_name + "_block");
        // if (!m_isProcedure)
        // {
        //     m_body->addVariableDefinition(VariableDefinition{.variableType = m_returnType,
        //                                                      .variableName = m_name,
        //                                                      .scopeId = 0,
        //                                                      .value = nullptr,
        //                                                      .constant = false});
        // }
        m_body->codegen(context);
        if (m_isProcedure)
        {
            context->Builder->CreateRetVoid();

            verifyFunction(*functionDefinition);
            if (context->compilerOptions.buildMode == BuildMode::Release)
            {
                context->TheFPM->run(*functionDefinition, *context->TheFAM);
            }

            return functionDefinition;
        }
        else
        {
            context->Builder->CreateRet(context->Builder->CreateLoad(
                    context->NamedAllocations[m_name]->getAllocatedType(), context->NamedAllocations[m_name]));
        }
        // Finish off the function.

        // Validate the generated code, checking for consistency.
        llvm::verifyFunction(*functionDefinition);
        if (context->compilerOptions.buildMode == BuildMode::Release)
        {
            context->TheFPM->run(*functionDefinition, *context->TheFAM);
        }
    }


    return functionDefinition;
}
void FunctionDefinitionNode::typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    if (m_body)
        m_body->typeCheck(unit, this);
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
    if (!m_libName.empty())
        return m_externalName;

    auto result = m_name + "(";
    for (size_t i = 0; i < m_params.size(); ++i)
    {
        result += m_params[i].type->typeName + ((i < m_params.size() - 1) ? "," : "");
    }
    result += ")";
    return result;
}

std::string &FunctionDefinitionNode::externalName() { return m_externalName; }

std::string &FunctionDefinitionNode::libName() { return m_libName; }
