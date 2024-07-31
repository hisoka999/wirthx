#pragma once
#include <string>
#include "ASTNode.h"

class InputNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_outputTextNode;
    std::string m_variableName;

public:
    InputNode(std::shared_ptr<ASTNode> outputTextNode, const std::string_view variableName);
    ~InputNode() = default;
    void print() override;
    void eval(InterpreterContext &context, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
