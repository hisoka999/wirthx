#include "ast/UnitNode.h"
#include <iostream>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>

#include "compare.h"
#include "compiler/Context.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "types/FileType.h"


UnitNode::UnitNode(const Token &token, const UnitType unitType, const std::string &unitName,
                   const std::vector<std::shared_ptr<FunctionDefinitionNode>> &functionDefinitions,
                   const std::unordered_map<std::string, std::shared_ptr<VariableType>> &typeDefinitions,
                   const std::shared_ptr<BlockNode> &blockNode) :
    ASTNode(token), m_unitType(unitType), m_unitName(unitName), m_functionDefinitions(functionDefinitions),
    m_typeDefinitions(typeDefinitions), m_blockNode(blockNode)
{
}
UnitNode::UnitNode(const Token &token, UnitType unitType, const std::string &unitName,
                   const std::vector<std::string> &argumentNames,
                   const std::vector<std::shared_ptr<FunctionDefinitionNode>> &functionDefinitions,
                   const std::unordered_map<std::string, std::shared_ptr<VariableType>> &typeDefinitions,
                   const std::shared_ptr<BlockNode> &blockNode) :
    ASTNode(token), m_unitType(unitType), m_unitName(unitName), m_functionDefinitions(functionDefinitions),
    m_typeDefinitions(typeDefinitions), m_blockNode(blockNode), m_argumentNames(argumentNames)
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
        if (iequals(def->functionSignature(), functionName) ||
            (iequals(def->name(), functionName) && def->externalName() != def->name()))
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

    m_blockNode->codegenConstantDefinitions(context);
    {
        //         @stderr = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
        // @.str = private unnamed_addr constant [9 x i8] c"MY ERROR\00", align 1
        // @stdout = external dso_local local_unnamed_addr global %struct._IO_FILE*, align 8
        auto cFile = llvm::PointerType::getUnqual(*context->TheContext);
        auto fileType = FileType::getFileType();
        auto ext_stderr = new llvm::GlobalVariable(*context->TheModule, cFile, false,
                                                   llvm::GlobalValue::ExternalLinkage, nullptr, "stderr");
        // ext_stderr->setExternallyInitialized(true);
        ext_stderr->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Local);
        if (m_argumentNames.size() >= 3)
        {
            m_blockNode->addVariableDefinition(VariableDefinition{.variableType = fileType,
                                                                  .variableName = m_argumentNames[2],
                                                                  .scopeId = 0,
                                                                  .llvmValue = ext_stderr,
                                                                  .constant = false});
        }

        context->NamedValues["stderr"] = ext_stderr;
        auto ext_stdout = new llvm::GlobalVariable(*context->TheModule, cFile, false,
                                                   llvm::GlobalValue::ExternalLinkage, nullptr, "stdout");
        // ext_stdout->setExternallyInitialized(true);
        context->NamedValues["stdout"] = ext_stdout;
        if (m_argumentNames.size() >= 2)
        {
            m_blockNode->addVariableDefinition(VariableDefinition{.variableType = fileType,
                                                                  .variableName = m_argumentNames[1],
                                                                  .scopeId = 0,
                                                                  .llvmValue = ext_stdout,
                                                                  .constant = false});
        }
        auto ext_stdin = new llvm::GlobalVariable(*context->TheModule, cFile, false, llvm::GlobalValue::ExternalLinkage,
                                                  nullptr, "stdin");
        // ext_stdin->setExternallyInitialized(true);
        context->NamedValues["stdin"] = ext_stdin;

        if (m_argumentNames.size() >= 1)
        {
            m_blockNode->addVariableDefinition(VariableDefinition{.variableType = fileType,
                                                                  .variableName = m_argumentNames[1],
                                                                  .scopeId = 0,
                                                                  .llvmValue = ext_stdin,
                                                                  .constant = false});
        }
    }

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
void UnitNode::typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode)
{
    for (const auto &def: m_functionDefinitions)
    {
        def->typeCheck(unit, parentNode);
    }

    m_blockNode->typeCheck(unit, parentNode);
}
