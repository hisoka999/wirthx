#include "BlockNode.h"
#include "compiler/Context.h"
#include <iostream>

BlockNode::BlockNode(std::vector<VariableDefinition> variableDefinitions, const std::vector<std::shared_ptr<ASTNode>> &expressions) : m_expressions(expressions), m_variableDefinitions(variableDefinitions)
{
}

void BlockNode::print()
{
    if (!m_variableDefinitions.empty())
        std::cout << "var\n";
    for (auto &def : m_variableDefinitions)
    {
        std::cout << def.variableName << " : " << def.variableType.typeName << ";\n";
    }
    std::cout << "begin\n";
    for (auto exp : m_expressions)
    {
        exp->print();
    }

    std::cout << "end;\n";
}

void BlockNode::eval(Stack &stack, std::ostream &outputStream)
{
    for (auto &exp : m_expressions)
    {
        exp->eval(stack, outputStream);
    }
}

void BlockNode::setBlockName(const std::string &name)
{
    m_blockname = name;
}

llvm::Value *BlockNode::codegen(std::unique_ptr<Context> &context)
{
    llvm::Function *TheFunction = context->TopLevelFunction;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock *BB = llvm::BasicBlock::Create(*context->TheContext, m_blockname, TheFunction);

    for (auto &def : m_variableDefinitions)
    {

        context->NamedValues[def.variableName] = def.generateCode(context);
    }
    std::vector<llvm::Value *> values;
    context->Builder->SetInsertPoint(BB);

    for (auto &exp : m_expressions)
    {
        values.push_back(exp->codegen(context));
    }

    for (auto &def : m_variableDefinitions)
    {

        context->NamedValues[def.variableName] = nullptr;
    }
    return llvm::Constant::getNullValue(llvm::Type::getDoubleTy(*context->TheContext));
}