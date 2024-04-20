#pragma once
#include "ASTNode.h"
#include <string>

class InputNode : public ASTNode
{
private:
    std::shared_ptr<ASTNode> m_outputTextNode;
    std::string m_variableName;

public:
    InputNode(std::shared_ptr<ASTNode> outputTextNode, const std::string_view variableName);
    ~InputNode() = default;
    void print() override;
    void eval(Stack &stack) override;
};
