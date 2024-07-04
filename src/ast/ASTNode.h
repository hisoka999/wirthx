#pragma once
#include <memory>
#include <vector>

namespace llvm
{
    class Value;

};
class Context;
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
};
