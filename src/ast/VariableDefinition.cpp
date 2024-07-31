#include "VariableDefinition.h"
#include "compare.h"
#include "compiler/Context.h"
using namespace std::literals;

llvm::AllocaInst *VariableDefinition::generateCode(std::unique_ptr<Context> &context) const
{
    llvm::Function *TheFunction = context->TopLevelFunction;
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(),
                           TheFunction->getEntryBlock().begin());

    auto array = std::dynamic_pointer_cast<ArrayType>(this->variableType);
    if (array != nullptr)
    {

        auto arraySize = array->heigh - array->low + 1;
        auto type = array->generateLlvmType(context);

        return TmpB.CreateAlloca(llvm::ArrayType::get(type, arraySize), nullptr, this->variableName);
    }

    switch (this->variableType->baseType)
    {
    case VariableBaseType::Integer:
    {
        auto allocation = TmpB.CreateAlloca(llvm::Type::getInt64Ty(*context->TheContext), nullptr,
                                            this->variableName);
        TmpB.CreateStore(TmpB.getInt64(0), allocation);
        return allocation;
    }
    case VariableBaseType::Float:
    case VariableBaseType::Real:
        return TmpB.CreateAlloca(llvm::Type::getDoubleTy(*context->TheContext), nullptr,
                                 this->variableName);
    default:
        return nullptr;
    }
}