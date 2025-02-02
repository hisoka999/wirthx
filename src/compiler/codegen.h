//
// Created by stefan on 01.02.25.
//

#ifndef CODEGEN_H
#define CODEGEN_H

#include <functional>
#include <memory>
namespace llvm
{
    class Value;

};
struct Context;

namespace codegen
{
    llvm::Value *codegen_ifexpr(std::unique_ptr<Context> &context, llvm::Value *condition,
                                std::function<void(std::unique_ptr<Context> &)> body);
}

#endif // CODEGEN_H
