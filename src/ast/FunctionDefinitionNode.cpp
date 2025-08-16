#include "FunctionDefinitionNode.h"
#include <iostream>
#include <llvm/IR/IRBuilder.h>
#include <utility>

#include "FieldAccessNode.h"
#include "UnitNode.h"
#include "compare.h"
#include "compiler/Context.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "types/ClassType.h"
#include "types/RecordType.h"


FunctionDefinitionNode::FunctionDefinitionNode(const Token &token, std::string name,
                                               const std::vector<FunctionArgument> &params,
                                               std::shared_ptr<BlockNode> body, const FunctionType functionType,
                                               std::optional<Token> parent, std::shared_ptr<VariableType> returnType) :
    ASTNode(token), m_name(std::move(name)), m_externalName(m_name), m_params(params), m_body(std::move(body)),
    m_functionType(functionType), m_returnType(std::move(returnType)), m_parent(std::move(parent))
{
}

FunctionDefinitionNode::FunctionDefinitionNode(const Token &token, std::string name, std::string externalName,
                                               std::string libName, const std::vector<FunctionArgument> &params,
                                               const FunctionType functionType, std::optional<Token> parent,
                                               std::shared_ptr<VariableType> returnType) :
    ASTNode(token), m_name(std::move(name)), m_externalName(std::move(externalName)), m_libName(std::move(libName)),
    m_params(params), m_body(nullptr), m_functionType(functionType), m_returnType(std::move(returnType)),
    m_parent(std::move(parent))
{
}

void FunctionDefinitionNode::print() {}

std::string &FunctionDefinitionNode::name() { return m_name; }

std::shared_ptr<VariableType> FunctionDefinitionNode::returnType() { return m_returnType; }

llvm::Value *FunctionDefinitionNode::codegen(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;

    if (m_parent)
    {
        auto classType = context->programUnit()->getTypeDefinitions().getType(m_parent.value().lexical());
        params.push_back(classType.value()->generateLlvmType(context)->getPointerTo());
    }

    for (const auto &param: m_params)
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
    if (m_functionType == FunctionType::Procedure or m_functionType == FunctionType::Constructor)
    {
        resultType = llvm::Type::getVoidTy(*context->context());
    }

    else
    {
        resultType = m_returnType->generateLlvmType(context);
    }
    llvm::FunctionType *FT = llvm::FunctionType::get(resultType, params, false);
    auto linkage = llvm::Function::ExternalLinkage;
    if (!m_libName.empty())
    {
        linkage = llvm::Function::ExternalLinkage;
    }
    else if (m_functionType == FunctionType::Constructor || m_functionType == FunctionType::Destructor)
    {
        linkage = llvm::Function::LinkOnceODRLinkage;
    }
    else
    {
        linkage = llvm::Function::InternalLinkage;
    }

    llvm::Function *functionDefinition = context->module()->getFunction(functionSignature());
    if (!functionDefinition)
        functionDefinition = llvm::Function::Create(FT, linkage, functionSignature(), context->module().get());

    // Set names for all arguments.
    unsigned idx = 0;
    size_t offset = 0;
    if (m_parent)
    {
        offset = 1;
    }
    for (auto &arg: functionDefinition->args())
    {
        if (arg.getArgNo() == 0 && offset == 1)
        {
            arg.setName("self");
            // noundef nonnull align 4 dereferenceable(4)
            arg.addAttr(llvm::Attribute::get(*context->context(), llvm::Attribute::NoUndef));
            arg.addAttr(llvm::Attribute::get(*context->context(), llvm::Attribute::NonNull));
            arg.addAttr(llvm::Attribute::get(*context->context(), llvm::Attribute::Alignment, 4));
            auto classType = context->programUnit()->getTypeDefinitions().getType(m_parent.value().lexical());
            auto llvmClassType = classType.value()->generateLlvmType(context);
            const llvm::DataLayout &DL = context->module()->getDataLayout();
            arg.addAttr(llvm::Attribute::getWithDereferenceableBytes(*context->context(),
                                                                     DL.getTypeAllocSize(llvmClassType)));
            continue;
        }
        const auto param = m_params[idx];
        if (!param.isReference && param.type->baseType == VariableBaseType::Struct)
        {
            arg.addAttr(llvm::Attribute::getWithByValType(*context->context(), param.type->generateLlvmType(context)));
            arg.addAttr(llvm::Attribute::NoUndef);
        }


        arg.setName(param.argumentName);
        idx++;
    }
    if (m_libName.empty())
    {
        // functionDefinition->setDSOLocal(true);
        functionDefinition->addFnAttr(llvm::Attribute::MustProgress);
        if (m_functionType != FunctionType::Procedure && m_returnType &&
            m_returnType->baseType == VariableBaseType::String)
            functionDefinition->addFnAttr(llvm::Attribute::NoFree);

        llvm::AttrBuilder b(*context->context());
        b.addAttribute("frame-pointer", "all");
        functionDefinition->addFnAttrs(b);
    }
    for (const auto attribute: m_attributes)
    {
        switch (attribute)
        {
            case FunctionAttribute::Inline:
                functionDefinition->addFnAttr(llvm::Attribute::AlwaysInline);
                break;
        }
    }


    context->addFunctionDefinition(functionSignature(), functionDefinition);
    // Create a new basic block to start insertion into.

    context->setCurrentFunction(functionDefinition);
    if (m_body)
    {
        context->explicitReturn = false;
        m_body->setBlockName(m_name + "_block");
        if (m_parent)
        {
            auto type = context->programUnit()->getTypeDefinitions().getType(m_parent->lexical());
            if (!type.has_value())
            {
                return LogErrorV("Unknown type for constructor: " + m_parent->lexical());
            }
            // m_body->addVariableDefinition(VariableDefinition{.variableType = type.value(),
            //                                                  .variableName = "self",
            //                                                  .token = ASTNode::expressionToken(),
            //                                                  .alias = "",
            //                                                  .scopeId = 0,
            //                                                  .llvmValue = nullptr,
            //                                                  .constant = false});
        }
        m_body->codegen(context);
        if (m_functionType == FunctionType::Procedure)
        {
            context->builder()->CreateRetVoid();

            context->verifyFunction(functionDefinition);
            return functionDefinition;
        }
        if (!context->explicitReturn && m_returnType)
        {
            context->builder()->CreateRet(
                    context->builder()->CreateLoad(resultType, context->namedAllocation(m_name), m_name));
        }
        else if (!m_returnType)
        {
            context->builder()->CreateRetVoid();
        }

        // Finish off the function.

        // Validate the generated code, checking for consistency.
        context->verifyFunction(functionDefinition);
    }


    return functionDefinition;
}
void FunctionDefinitionNode::typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    if (m_body)
        m_body->typeCheck(unit, this);
}
void FunctionDefinitionNode::addAttribute(FunctionAttribute attribute) { m_attributes.emplace_back(attribute); }
FunctionType FunctionDefinitionNode::functionType() const { return m_functionType; }
std::optional<std::string> FunctionDefinitionNode::parent() const
{
    return m_parent.has_value() ? std::make_optional(m_parent.value().lexical()) : std::nullopt;
}

std::optional<FunctionArgument> FunctionDefinitionNode::getParam(const std::string &paramName) const
{
    for (auto &param: m_params)
    {
        if (iequals(param.argumentName, paramName))
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

std::shared_ptr<BlockNode> FunctionDefinitionNode::body() const { return m_body; }


std::string FunctionDefinitionNode::functionSignature()
{
    if (!m_libName.empty())
        return m_externalName;
    if (m_functionSignature.empty())
    {
        std::stringstream stream;
        if (m_parent)
        {
            stream << to_lower(m_parent->lexical()) << ".";
        }

        stream << to_lower(m_name) << "(";
        for (size_t i = 0; i < m_params.size(); ++i)
        {
            stream << m_params[i].type->typeName << ((i < m_params.size() - 1) ? "," : "");
        }
        stream << ")";
        m_functionSignature = stream.str();
    }
    return m_functionSignature;
}

std::string &FunctionDefinitionNode::externalName() { return m_externalName; }

std::string &FunctionDefinitionNode::libName() { return m_libName; }
