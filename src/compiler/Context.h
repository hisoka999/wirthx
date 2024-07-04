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
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

// namespace llvm
// {
//     class LLVMContext;
//     class Module;
//     template< typename T>
//     class IRBuilder<T>;
//     class Value;
// };

struct Context
{
    std::unique_ptr<llvm::LLVMContext> TheContext;
    std::unique_ptr<llvm::Module> TheModule;
    std::unique_ptr<llvm::IRBuilder<>> Builder;
    std::map<std::string, llvm::AllocaInst *> NamedValues;
    llvm::Function *TopLevelFunction;
    std::map<std::string, llvm::Function *> FunctionDefinitions;
};

void LogError(const char *Str);
llvm::Value *LogErrorV(const char *Str);