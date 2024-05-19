#pragma once
#include <memory>
#include <vector>

class Stack;

class ASTNode
{
public:
    ASTNode();
    virtual ~ASTNode(){};

    virtual void print() = 0;
    virtual void eval(Stack &stack, std::ostream &outputStream) = 0;
};
