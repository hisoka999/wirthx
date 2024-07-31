#pragma once
#include <string>
#include "ASTNode.h"

class VariableAccessNode : public ASTNode
{
private:
    std::string m_variableName;

public:
    VariableAccessNode(const std::string_view variableName);
    ~VariableAccessNode() = default;
    void print() override;
    void eval(InterpreterContext &context, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unit, ASTNode *parent) override;
};
