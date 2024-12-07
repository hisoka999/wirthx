#include "ast/VariableType.h"
#include <cassert>
#include "compiler/Context.h"

VariableType::VariableType(VariableBaseType baseType, std::string typeName) : baseType(baseType), typeName(typeName) {}

llvm::Type *VariableType::generateLlvmType(std::unique_ptr<Context> &context)
{
    switch (this->baseType)
    {
        case VariableBaseType::Float:
        case VariableBaseType::Real:
            return llvm::Type::getDoubleTy(*context->TheContext);
        case VariableBaseType::String:
            return llvm::PointerType::getUnqual(*context->TheContext);
        case VariableBaseType::Boolean:
            return llvm::Type::getInt1Ty(*context->TheContext);
        default:
            assert(false && "unknown base type to generate llvm type for");
    }
}

std::shared_ptr<IntegerType> VariableType::getInteger(size_t length)
{
    auto integer = std::make_shared<IntegerType>();
    integer->baseType = VariableBaseType::Integer;
    integer->length = length;
    integer->typeName = "integer" + std::to_string(length);
    return integer;
}

std::shared_ptr<VariableType> VariableType::getBoolean()
{
    auto boolean = std::make_shared<VariableType>();
    boolean->baseType = VariableBaseType::Boolean;
    boolean->typeName = "boolean";
    return boolean;
}

std::shared_ptr<VariableType> VariableType::getString()
{
    return std::make_shared<VariableType>(VariableBaseType::String, "string");
}

std::shared_ptr<ArrayType> ArrayType::getFixedArray(size_t low, size_t heigh,
                                                    const std::shared_ptr<VariableType> &baseType)
{
    auto type = std::make_shared<ArrayType>();
    type->baseType = VariableBaseType::Array;
    type->low = low;
    type->high = heigh;
    type->arrayBase = baseType;
    type->isDynArray = false;
    return type;
}

std::shared_ptr<ArrayType> ArrayType::getDynArray(const std::shared_ptr<VariableType> &baseType)
{
    auto type = std::make_shared<ArrayType>();
    type->baseType = VariableBaseType::Array;
    type->low = 0;
    type->high = 0;
    type->arrayBase = baseType;
    type->isDynArray = true;
    return type;
}


llvm::Type *IntegerType::generateLlvmType(std::unique_ptr<Context> &context)
{
    return llvm::IntegerType::get(*context->TheContext, this->length);
}


llvm::Type *ArrayType::generateLlvmType(std::unique_ptr<Context> &context)
{

    if (llvmType == nullptr)
    {
        auto arrayBaseType = arrayBase->generateLlvmType(context);
        if (isDynArray)
        {

            std::vector<llvm::Type *> types;
            types.emplace_back(VariableType::getInteger(64)->generateLlvmType(context));

            types.emplace_back(llvm::PointerType::getUnqual(arrayBaseType));


            llvm::ArrayRef<llvm::Type *> Elements(types);


            llvmType = llvm::StructType::create(Elements);
        }
        else
        {

            auto arraySize = high - low + 1;

            llvmType = llvm::ArrayType::get(arrayBaseType, arraySize);
        }
    }
    return llvmType;
}
