#include "ast/UnitNode.h"
#include "compiler/Context.h"
#include <iostream>

UnitNode::UnitNode(UnitType unitType, const std::string unitName, std::vector<std::shared_ptr<FunctionDefinitionNode>> functionDefinitions, const std::shared_ptr<BlockNode> &blockNode)
    : m_unitType(unitType), m_unitName(unitName), m_functionDefinitions(functionDefinitions), m_blockNode(blockNode)
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
    for (auto def : m_functionDefinitions)
    {
        def->print();
    }

    m_blockNode->print();
}

void UnitNode::eval(Stack &stack, std::ostream &outputStream)
{
    m_blockNode->eval(stack, outputStream);
}

std::vector<std::shared_ptr<FunctionDefinitionNode>> UnitNode::getFunctionDefinitions()
{
    return m_functionDefinitions;
}

void UnitNode::addFunctionDefinition(const std::shared_ptr<FunctionDefinitionNode> &functionDefinition)
{
    m_functionDefinitions.push_back(functionDefinition);
}

std::string UnitNode::getUnitName()
{
    return m_unitName;
}

llvm::Value *UnitNode::codegen(std::unique_ptr<Context> &context)
{
    std::vector<llvm::Type *> params;

    for (auto &fdef : m_functionDefinitions)
    {
        fdef->codegen(context);
    }
    llvm::FunctionType *FT =
        llvm::FunctionType::get(llvm::Type::getInt32Ty(*context->TheContext), params, false);

    llvm::Function *F =
        llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "_start", context->TheModule.get());

    context->TopLevelFunction = F;
    m_blockNode->setBlockName("entry");
    // Create a new basic block to start insertion into.
    m_blockNode->codegen(context);
    verifyFunction(*F, &llvm::errs());

    llvm::Function *exitCall = context->TheModule->getFunction("exit");
    std::vector<llvm::Value *> exitArgs;
    exitArgs.push_back(llvm::ConstantInt::get(*context->TheContext, llvm::APInt(32, 0)));

    context->Builder->CreateCall(exitCall, exitArgs);

    context->Builder->CreateRet(llvm::ConstantInt::get(*context->TheContext, llvm::APInt(32, 0)));

    return nullptr;
}