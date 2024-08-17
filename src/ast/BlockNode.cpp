#include "BlockNode.h"
#include <iostream>
#include "compiler/Context.h"
#include "interpreter/InterpreterContext.h"

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

void BlockNode::eval(InterpreterContext &context, std::ostream &outputStream)
{
    for (auto &def: m_variableDefinitions)
    {
        if (def.value)
        {
            def.value->eval(context, outputStream);
            if (def.variableType->baseType == VariableBaseType::Integer ||
                def.variableType->baseType == VariableBaseType::Boolean)
            {
                context.stack.set_var(def.variableName, context.stack.pop_front<int64_t>());
            }
            else if (def.variableType->baseType == VariableBaseType::String)
            {
                context.stack.set_var(def.variableName, context.stack.pop_front<std::string_view>());
            }
        }
    }

    for (auto &exp: m_expressions)
    {
        exp->eval(context, outputStream);
    }
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

    for (auto &exp: m_expressions)
    {
        values.push_back(exp->codegen(context));
    }

    for (auto &def: m_variableDefinitions)
    {

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
    return std::nullopt;
}
