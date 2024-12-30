#include "VariableType.h"
#include <cassert>
#include <llvm/IR/IRBuilder.h>

#include "compiler/Context.h"
#include "exceptions/CompilerException.h"

VariableType::VariableType(const VariableBaseType baseType, const std::string &typeName) :
    baseType(baseType), typeName(typeName)
{
}
bool VariableType::isSimpleType() const
{
    switch (this->baseType)
    {
        case VariableBaseType::Array:
        case VariableBaseType::Struct:
        case VariableBaseType::String:
            return false;
        case VariableBaseType::Pointer:
        case VariableBaseType::Integer:
        case VariableBaseType::Float:
        case VariableBaseType::Real:;
        case VariableBaseType::Boolean:
            return true;
        default:
            assert(false && "unknown base type to generate llvm type for");
            return false;
    }
}

llvm::Type *VariableType::generateLlvmType(std::unique_ptr<Context> &context)
{
    switch (this->baseType)
    {
        case VariableBaseType::Pointer:
            return llvm::PointerType::getUnqual(*context->TheContext);
        case VariableBaseType::Float:
        case VariableBaseType::Real:
            return llvm::Type::getDoubleTy(*context->TheContext);
        case VariableBaseType::Boolean:
            return llvm::Type::getInt1Ty(*context->TheContext);
        default:
            assert(false && "unknown base type to generate llvm type for");
            return nullptr;
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

std::shared_ptr<VariableType> VariableType::getPointer()
{
    auto pointer = std::make_shared<VariableType>();
    pointer->baseType = VariableBaseType::Pointer;
    pointer->typeName = "pointer";
    return pointer;
}
bool VariableType::operator==(const VariableType &other) const { return this->baseType == other.baseType; }


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

llvm::Value *ArrayType::generateFieldAccess(Token &token, llvm::Value *indexValue, std::unique_ptr<Context> &context)
{
    auto arrayName = std::string(token.lexical());
    llvm::AllocaInst *V = context->NamedAllocations[arrayName];

    if (!V)
        return LogErrorV("Unknown variable for array access: " + arrayName);

    if (this->isDynArray)
    {
        auto llvmRecordType = this->generateLlvmType(context);
        auto arrayBaseType = this->arrayBase->generateLlvmType(context);

        auto arrayPointerOffset = context->Builder->CreateStructGEP(llvmRecordType, V, 1, "array.ptr.offset");
        // const llvm::DataLayout &DL = context->TheModule->getDataLayout();
        // auto alignment = DL.getPrefTypeAlign(ptrType);
        auto loadResult =
                context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), arrayPointerOffset);


        auto bounds = context->Builder->CreateGEP(arrayBaseType, loadResult, llvm::ArrayRef<llvm::Value *>{indexValue},
                                                  "", true);

        return context->Builder->CreateLoad(arrayBaseType, bounds);
    }

    if (llvm::isa<llvm::ConstantInt>(indexValue))
    {
        llvm::ConstantInt *value = reinterpret_cast<llvm::ConstantInt *>(indexValue);

        if (value->getSExtValue() < static_cast<int64_t>(this->low) ||
            value->getSExtValue() > static_cast<int64_t>(this->high))
        {
            throw CompilerException(
                    ParserError{.token = token, .message = "the array index is not in the defined range."});
        }
    }
    llvm::Value *index = indexValue;
    if (this->low > 0)
        index = context->Builder->CreateSub(
                index, context->Builder->getIntN(index->getType()->getIntegerBitWidth(), this->low), "array.index.sub");


    auto arrayValue = context->Builder->CreateGEP(V->getAllocatedType(), V, {context->Builder->getInt64(0), index},
                                                  "arrayindex", false);
    return context->Builder->CreateLoad(V->getAllocatedType()->getArrayElementType(), arrayValue);
}

std::shared_ptr<PointerType> PointerType::getPointerTo(const std::shared_ptr<VariableType> &baseType)
{
    auto ptrType = std::make_shared<PointerType>();

    ptrType->pointerBase = baseType;
    ptrType->baseType = VariableBaseType::Pointer;
    ptrType->typeName = baseType->typeName + "_ptr";
    return ptrType;
}
std::shared_ptr<PointerType> PointerType::getUnqual()
{
    auto ptrType = std::make_shared<PointerType>();
    ptrType->pointerBase = nullptr;
    ptrType->baseType = VariableBaseType::Pointer;
    ptrType->typeName = "pointer";
    return ptrType;
}
llvm::Type *PointerType::generateLlvmType(std::unique_ptr<Context> &context)
{
    if (pointerBase)
    {
        const auto llvmBaseType = pointerBase->generateLlvmType(context);
        return llvm::PointerType::getUnqual(llvmBaseType);
    }
    return llvm::PointerType::getUnqual(*context->TheContext);
}
