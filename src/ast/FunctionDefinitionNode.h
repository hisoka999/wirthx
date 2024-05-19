#pragma once

#include "ASTNode.h"
#include "VariableType.h"
#include <string>
#include <vector>

struct FunctionArgument
{
    VariableType type;
    std::string argumentName;
    bool isReference;
};

class FunctionDefinitionNode : public ASTNode
{
private:
    std::string m_name;
    std::vector<FunctionArgument> m_params;
    std::vector<std::shared_ptr<ASTNode>> m_body;
    bool m_isProcedure;
    VariableType m_returnType;

public:
    FunctionDefinitionNode(std::string name, std::vector<FunctionArgument> params, std::vector<std::shared_ptr<ASTNode>> body, bool isProcedure, VariableType returnType = VariableType());
    ~FunctionDefinitionNode() = default;
    void print() override;
    void eval(Stack &stack, std::ostream &outputStream) override;
    std::string &name();
};
