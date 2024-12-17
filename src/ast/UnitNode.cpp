#include "ast/UnitNode.h"
#include <iostream>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

#include "compiler/Context.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"


UnitNode::UnitNode(UnitType unitType, const std::string unitName,
                   std::vector<std::shared_ptr<FunctionDefinitionNode>> functionDefinitions,
                   std::map<std::string, std::shared_ptr<VariableType>> typeDefinitions,
                   const std::shared_ptr<BlockNode> &blockNode) :
    m_unitType(unitType), m_unitName(unitName), m_functionDefinitions(functionDefinitions),
    m_typeDefinitions(typeDefinitions), m_blockNode(blockNode)
{
}

void UnitNode::print()
{
    if (m_unitType == UnitType::PROGRAM)
    {
        std::cout << "program ";
    }
    else
    {
        std::cout << "unit ";
    }
    std::cout << m_unitName << "\n";
    for (auto def: m_functionDefinitions)
    {
        def->print();
    }

    m_blockNode->print();
}

std::vector<std::shared_ptr<FunctionDefinitionNode>> UnitNode::getFunctionDefinitions()
{
    return m_functionDefinitions;
}

std::optional<std::shared_ptr<FunctionDefinitionNode>> UnitNode::getFunctionDefinition(const std::string &functionName)
{
    for (auto &def: m_functionDefinitions)
    {
        if (def->functionSignature() == functionName ||
            (def->name() == functionName && def->externalName() != def->name()))
        {
            return def;
        }
    }
    return std::nullopt;
}

void UnitNode::addFunctionDefinition(const std::shared_ptr<FunctionDefinitionNode> &functionDefinition)
{
    m_functionDefinitions.push_back(functionDefinition);
}

std::string UnitNode::getUnitName() { return m_unitName; }

llvm::Value *UnitNode::codegen(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;

    for (auto &fdef: m_functionDefinitions)
    {
        fdef->codegen(context);
    }
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getInt32Ty(*context->TheContext), params, false);

    std::string functionName = m_unitName;
    if (m_unitType == UnitType::PROGRAM)
    {
        functionName = "main";
    }

    llvm::Function *F =
            llvm::Function::Create(FT, llvm::Function::ExternalLinkage, functionName, context->TheModule.get());
    context->TopLevelFunction = F;
    m_blockNode->setBlockName("entry");
    // Create a new basic block to start insertion into.
    m_blockNode->codegen(context);

    llvm::Function *exitCall = context->TheModule->getFunction("exit");
    std::vector<llvm::Value *> exitArgs;
    exitArgs.push_back(llvm::ConstantInt::get(*context->TheContext, llvm::APInt(32, 0)));

    context->Builder->CreateCall(exitCall, exitArgs);

    context->Builder->CreateRet(llvm::ConstantInt::get(*context->TheContext, llvm::APInt(32, 0)));
    verifyFunction(*F, &llvm::errs());
    if (context->compilerOptions.buildMode == BuildMode::Release)
    {
        context->TheFPM->run(*F, *context->TheFAM);
    }
    return nullptr;
}

std::optional<VariableDefinition> UnitNode::getVariableDefinition(const std::string &name)
{
    return m_blockNode->getVariableDefinition(name);
}

std::set<std::string> UnitNode::collectLibsToLink()
{
    std::set<std::string> result;
    result.insert("c");
    for (auto &function: m_functionDefinitions)
    {
        auto libName = function->libName();
        if (!libName.empty())
            result.insert(libName);
    }
    return result;
}
