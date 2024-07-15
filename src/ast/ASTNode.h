#pragma once
#include "VariableType.h"
#include <memory>
#include <vector>
namespace llvm
{
    class Value;

};
struct Context;
// static std::unique_ptr<LLVMContext> TheContext;

class Stack;

class ASTNode
{
public:
    ASTNode();
    virtual ~ASTNode() {};

    virtual void print() = 0;
    virtual void eval(Stack &stack, std::ostream &outputStream) = 0;
    virtual llvm::Value *codegen(std::unique_ptr<Context> &context) = 0;

    virtual VariableType resolveType(std::unique_ptr<Context> &context);
};