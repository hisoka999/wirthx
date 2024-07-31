#pragma once

#include "Stack.h"

class UnitNode;
class ASTNode;

struct InterpreterContext
{
    Stack stack;
    std::unique_ptr<UnitNode> unit;
    ASTNode *parent = nullptr;
};
