#pragma once

#include "ASTNode.h"
#include <string>
#include <vector>

class FunctionCallNode : public ASTNode
{
protected:
    std::string m_name;
    std::vector<std::shared_ptr<ASTNode>> m_args;

public:
    FunctionCallNode(std::string name, std::vector<std::shared_ptr<ASTNode>> args);
    ~FunctionCallNode() = default;
    void print() override;
    void eval(Stack &stack, std::ostream &outputStream) override;
};
