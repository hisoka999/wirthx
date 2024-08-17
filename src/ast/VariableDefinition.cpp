#include "VariableDefinition.h"
#include "ASTNode.h"
#include "compare.h"
#include "compiler/Context.h"
using namespace std::literals;

llvm::AllocaInst *VariableDefinition::generateCode(std::unique_ptr<Context> &context) const
{
    llvm::Function *TheFunction = context->TopLevelFunction;
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());

    auto array = std::dynamic_pointer_cast<ArrayType>(this->variableType);
    if (array != nullptr)
    {

        auto arraySize = array->high - array->low + 1;
        auto type = array->arrayBase->generateLlvmType(context);

        return TmpB.CreateAlloca(llvm::ArrayType::get(type, arraySize), nullptr, this->variableName);
    }

    switch (this->variableType->baseType)
    {
        case VariableBaseType::Integer:
        {
            auto type = this->variableType->generateLlvmType(context);
            auto allocation = TmpB.CreateAlloca(type, nullptr, this->variableName);
            TmpB.CreateStore(TmpB.getInt64(0), allocation);
            return allocation;
        }
        case VariableBaseType::Boolean:
        {
            auto allocation = TmpB.CreateAlloca(TmpB.getInt1Ty(), nullptr, this->variableName);
            TmpB.CreateStore(TmpB.getFalse(), allocation);
            return allocation;
        }
        case VariableBaseType::Float:
        case VariableBaseType::Real:
            return TmpB.CreateAlloca(llvm::Type::getDoubleTy(*context->TheContext), nullptr, this->variableName);
        default:
            return nullptr;
    }
}

llvm::Value *VariableDefinition::generateCodeForConstant(std::unique_ptr<Context> &context) const
{
    llvm::Function *TheFunction = context->TopLevelFunction;
    llvm::IRBuilder<> TmpB(&TheFunction->getEntryBlock(), TheFunction->getEntryBlock().begin());

    auto array = std::dynamic_pointer_cast<ArrayType>(this->variableType);

    return this->value->codegen(context);
}
