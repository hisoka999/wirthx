#pragma once

#include <cstddef>
#include <string>
#include "ASTNode.h"
#include "Token.h"

class ArrayAccessNode : public ASTNode
{
private:
    TokenWithFile m_arrayNameToken;
    std::string m_arrayName;
    std::shared_ptr<ASTNode> m_indexNode;

public:
    ArrayAccessNode(const TokenWithFile arrayName, const std::shared_ptr<ASTNode> &indexNode);
    ~ArrayAccessNode() = default;
    void print() override;
    void eval(InterpreterContext &context, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
};
