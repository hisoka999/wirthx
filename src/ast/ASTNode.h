#pragma once
#include <memory>
#include <vector>
#include "VariableType.h"
namespace llvm
{
    class Value;

};
struct Context;
// static std::unique_ptr<LLVMContext> TheContext;

struct InterpreterContext;

class UnitNode;

class ASTNode
{
public:
    ASTNode();
    virtual ~ASTNode() {};

    virtual void print() = 0;
    virtual void eval(InterpreterContext &context, std::ostream &outputStream) = 0;
    virtual llvm::Value *codegen(std::unique_ptr<Context> &context) = 0;

    virtual std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode);
};
