#pragma once
#include <map>
#include <memory>
#include <string>

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include "CompilerOptions.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
// namespace llvm
// {
//     class LLVMContext;
//     class Module;
//     template< typename T>
//     class IRBuilder<T>;
//     class Value;
// };

class UnitNode;

struct BreakBasicBlock
{
    llvm::BasicBlock *Block = nullptr;
    bool BlockUsed = false;
};

struct Context
{
    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::map<std::string, llvm::AllocaInst *> NamedAllocations;
    std::map<std::string, llvm::Value *> NamedValues;
    llvm::Function *TopLevelFunction;
    std::map<std::string, llvm::Function *> FunctionDefinitions;
    BreakBasicBlock BreakBlock;

    std::unique_ptr<llvm::FunctionPassManager> TheFPM;
    std::unique_ptr<llvm::LoopAnalysisManager> TheLAM;
    std::unique_ptr<llvm::FunctionAnalysisManager> TheFAM;
    std::unique_ptr<llvm::CGSCCAnalysisManager> TheCGAM;
    std::unique_ptr<llvm::ModuleAnalysisManager> TheMAM;
    std::unique_ptr<llvm::PassInstrumentationCallbacks> ThePIC;
    std::unique_ptr<llvm::StandardInstrumentations> TheSI;

    std::unique_ptr<UnitNode> ProgramUnit;
    CompilerOptions compilerOptions;
};

void LogError(const char *Str);
llvm::Value *LogErrorV(const char *Str);
llvm::Value *LogErrorV(const std::string &Str);
