#pragma once

#include "FunctionCallNode.h"

bool isKnownSystemCall(const std::string &name);

class SystemFunctionCallNode : public FunctionCallNode
{
private:
public:
    SystemFunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args);
    ~SystemFunctionCallNode() = default;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
