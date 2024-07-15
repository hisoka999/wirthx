#pragma once
#include "ASTNode.h"
#include <string>

class VariableAccessNode : public ASTNode
{
private:
    std::string m_variableName;

public:
    VariableAccessNode(const std::string_view variableName);
    ~VariableAccessNode() = default;
    void print() override;
    void eval(Stack &stack, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    VariableType resolveType(std::unique_ptr<Context> &context) override;
};
