#pragma once
#include <memory>
#include <optional>
#include <vector>
#include "VariableType.h"
namespace llvm
{
    class Value;

};
struct Context;


class UnitNode;

class ASTNode
{
public:
    ASTNode();
    virtual ~ASTNode() = default;

    virtual void print() = 0;
    virtual llvm::Value *codegen(std::unique_ptr<Context> &context) = 0;

    virtual std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode);
    virtual std::optional<std::shared_ptr<ASTNode>> block() { return std::nullopt; }
    virtual void typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) {};
};
