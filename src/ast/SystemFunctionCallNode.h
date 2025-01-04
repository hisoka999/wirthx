#pragma once

#include "FunctionCallNode.h"

bool isKnownSystemCall(const std::string &name);

class SystemFunctionCallNode : public FunctionCallNode
{
private:
public:
    SystemFunctionCallNode(const Token &token, std::string name, std::vector<std::shared_ptr<ASTNode>> args);
    llvm::Value *codegen_setlength(std::unique_ptr<Context> &context, ASTNode *parent);
    llvm::Value *codegen_length(std::unique_ptr<Context> &context, ASTNode *parent) const;
    llvm::Value *codegen_write(std::unique_ptr<Context> &context, ASTNode *parent);
    llvm::Value *codegen_writeln(std::unique_ptr<Context> &context, ASTNode *parent);
    llvm::Value *codegen_new(std::unique_ptr<Context> &context, ASTNode *parent) const;
    ~SystemFunctionCallNode() override = default;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
    std::shared_ptr<VariableType> resolveType(const std::unique_ptr<UnitNode> &unitNode, ASTNode *parentNode) override;
};
