#pragma once

#include <string>
#include <vector>
#include "ASTNode.h"
#include "BlockNode.h"
#include "VariableType.h"

struct FunctionArgument
{
    std::shared_ptr<VariableType> type;
    std::string argumentName;
    bool isReference;
};

class FunctionDefinitionNode : public ASTNode
{
private:
    std::string m_name;
    std::vector<FunctionArgument> m_params;
    std::shared_ptr<BlockNode> m_body;
    bool m_isProcedure;
    std::shared_ptr<VariableType> m_returnType;

public:
    FunctionDefinitionNode(std::string name, std::vector<FunctionArgument> params, std::shared_ptr<BlockNode> body,
                           bool isProcedure,
                           std::shared_ptr<VariableType> returnType = std::make_shared<VariableType>());
    ~FunctionDefinitionNode() = default;
    void print() override;
    std::string functionSignature();
    std::string &name();
    std::shared_ptr<VariableType> returnType();
    std::optional<FunctionArgument> getParam(const std::string &paramName);
    std::optional<FunctionArgument> getParam(const size_t index);
    std::shared_ptr<BlockNode> body();
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;
};
