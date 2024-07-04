#pragma once

#include "FunctionCallNode.h"

bool isKnownSystemCall(const std::string &name);

class SystemFunctionCallNode : public FunctionCallNode
{
private:
public:
    SystemFunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args);
    ~SystemFunctionCallNode() = default;
    void eval(Stack &stack, std::ostream &outputStream) override;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
