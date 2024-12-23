#include "BlockNode.h"
#include <iostream>
#include "compiler/Context.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"


BlockNode::BlockNode(std::vector<VariableDefinition> variableDefinitions,
                     const std::vector<std::shared_ptr<ASTNode>> &expressions) :
    m_expressions(expressions), m_variableDefinitions(variableDefinitions)
{
}

void BlockNode::print()
{
    if (!m_variableDefinitions.empty())
        std::cout << "var\n";
    for (auto &def: m_variableDefinitions)
    {
        std::cout << def.variableName << " : " << def.variableType->typeName << ";\n";
    }
    std::cout << "begin\n";
    for (auto exp: m_expressions)
    {
        exp->print();
    }

    std::cout << "end;\n";
}


void BlockNode::setBlockName(const std::string &name) { m_blockname = name; }

llvm::Value *BlockNode::codegen(std::unique_ptr<Context> &context)
{
    if (!m_blockname.empty())
    {

        // Create a new basic block to start insertion into.
        llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context->TheContext, m_blockname, context->TopLevelFunction);
        context->Builder->SetInsertPoint(BB);
    }
    for (auto &def: m_variableDefinitions)
    {
        if (def.constant)
        {
            context->NamedValues[def.variableName] = def.generateCodeForConstant(context);
        }
        else
        {
            context->NamedAllocations[def.variableName] = def.generateCode(context);
            if (def.value)
            {

                auto result = def.value->codegen(context);

                context->Builder->CreateStore(result, context->NamedAllocations[def.variableName]);
            }
        }
    }
    std::vector<llvm::Value *> values;

    for (const auto &exp: m_expressions)
    {
        values.push_back(exp->codegen(context));
    }

    auto topLevelFunctionName = (context->TopLevelFunction) ? context->TopLevelFunction->getName().str() : "";
    if (context->TopLevelFunction)
        topLevelFunctionName = topLevelFunctionName.substr(0, topLevelFunctionName.find('('));
    for (auto &def: m_variableDefinitions)
    {

        if (!context->TopLevelFunction || def.variableName != topLevelFunctionName)
            context->NamedAllocations[def.variableName] = nullptr;
    }
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*context->TheContext));
}

std::optional<VariableDefinition> BlockNode::getVariableDefinition(const std::string &name)
{
    for (auto def: m_variableDefinitions)
    {
        if (def.variableName == name)
        {
            return def;
        }
    }
    for (auto &node: m_expressions)
    {
        auto block = node->block();
        if (!block)
        {
            continue;
        }
        if (auto b = std::dynamic_pointer_cast<BlockNode>(block.value()))
        {
            auto result = b->getVariableDefinition(name);
            if (result)
            {
                return result;
            }
        }
    }

    return std::nullopt;
}

void BlockNode::addVariableDefinition(VariableDefinition definition) { m_variableDefinitions.emplace_back(definition); }


void BlockNode::appendExpression(std::shared_ptr<ASTNode> node) { m_expressions.push_back(node); }
void BlockNode::typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    for (auto exp: m_expressions)
    {
        exp->typeCheck(unit, parentNode);
    }
}

void BlockNode::preappendExpression(std::shared_ptr<ASTNode> node)
{
    m_expressions.emplace(m_expressions.begin(), node);
}
