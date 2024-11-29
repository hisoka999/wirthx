#include "VariableDefinition.h"
#include "ASTNode.h"
#include "RecordType.h"
#include "compare.h"
#include "compiler/Context.h"
using namespace std::literals;

llvm::AllocaInst *VariableDefinition::generateCode(std::unique_ptr<Context> &context) const
{
    auto array = std::dynamic_pointer_cast<ArrayType>(this->variableType);
    if (array != nullptr)
    {

        auto arraySize = array->high - array->low + 1;
        auto type = array->arrayBase->generateLlvmType(context);

        return context->Builder->CreateAlloca(llvm::ArrayType::get(type, arraySize), nullptr, this->variableName);
    }
    auto structType = std::dynamic_pointer_cast<RecordType>(this->variableType);
    if (structType != nullptr)
    {
        return context->Builder->CreateAlloca(structType->generateLlvmType(context), nullptr, this->variableName);
    }

    switch (this->variableType->baseType)
    {
        case VariableBaseType::Integer:
        {
            auto type = this->variableType->generateLlvmType(context);
            auto allocation = context->Builder->CreateAlloca(type, nullptr, this->variableName);
            context->Builder->CreateStore(context->Builder->getInt64(0), allocation);
            return allocation;
        }
        case VariableBaseType::Boolean:
        {
            auto allocation =
                    context->Builder->CreateAlloca(context->Builder->getInt1Ty(), nullptr, this->variableName);
            context->Builder->CreateStore(context->Builder->getFalse(), allocation);
            return allocation;
        }
        case VariableBaseType::Float:
        case VariableBaseType::Real:
            return context->Builder->CreateAlloca(llvm::Type::getDoubleTy(*context->TheContext), nullptr,
                                                  this->variableName);
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
