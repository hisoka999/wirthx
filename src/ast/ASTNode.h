#pragma once
#include <vector>
#include <memory>

class Stack;

class ASTNode
{
public:
    ASTNode();
    virtual ~ASTNode(){};

    virtual void print() = 0;
    virtual void eval(Stack &stack) = 0;
};
