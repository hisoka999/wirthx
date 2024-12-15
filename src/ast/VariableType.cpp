#include "ast/VariableType.h"
#include <cassert>
#include "compiler/Context.h"
#include "exceptions/CompilerException.h"

VariableType::VariableType(VariableBaseType baseType, std::string typeName) : baseType(baseType), typeName(typeName) {}

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

std::shared_ptr<StringType> VariableType::getString()
{
    static auto stringType = std::make_shared<StringType>();
    stringType->baseType = VariableBaseType::String;
    stringType->typeName = "string";
    return stringType;
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

llvm::Value *ArrayType::generateFieldAccess(TokenWithFile &token, llvm::Value *indexValue,
                                            std::unique_ptr<Context> &context)
{
    auto arrayName = std::string(token.token.lexical);
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
            throw CompilerException(ParserError{.file_name = token.fileName,
                                                .token = token.token,
                                                .message = "the array index is not in the defined range."});
        }
    }
    llvm::Value *index = indexValue;
    if (this->low > 0)
        index = context->Builder->CreateSub(
                index, context->Builder->getIntN(index->getType()->getIntegerBitWidth(), this->low), "array.index.sub");

    llvm::ArrayRef<llvm::Value *> idxList = {context->Builder->getInt64(0), index};

    auto arrayValue = context->Builder->CreateGEP(V->getAllocatedType(), V, idxList, "arrayindex", false);
    return context->Builder->CreateLoad(V->getAllocatedType()->getArrayElementType(), arrayValue);
}


llvm::Type *StringType::generateLlvmType(std::unique_ptr<Context> &context)
{
    if (llvmType == nullptr)
    {
        auto baseType = IntegerType::getInteger(8);
        auto charType = baseType->generateLlvmType(context);
        std::vector<llvm::Type *> types;
        types.emplace_back(VariableType::getInteger(64)->generateLlvmType(context));
        types.emplace_back(VariableType::getInteger(64)->generateLlvmType(context));
        types.emplace_back(llvm::PointerType::getUnqual(charType));


        llvm::ArrayRef<llvm::Type *> Elements(types);


        llvmType = llvm::StructType::create(Elements, "string");
    }
    return llvmType;
}

llvm::Value *StringType::generateFieldAccess(TokenWithFile &token, llvm::Value *indexValue,
                                             std::unique_ptr<Context> &context)
{
    auto arrayName = std::string(token.token.lexical);
    llvm::Value *V = context->NamedAllocations[arrayName];

    if (!V)
    {
        for (auto &arg: context->TopLevelFunction->args())
        {
            if (arg.getName() == arrayName)
            {
                V = context->TopLevelFunction->getArg(arg.getArgNo());
                break;
            }
        }
    }

    if (!V)
        return LogErrorV("Unknown variable for string access: " + arrayName);


    auto llvmRecordType = this->generateLlvmType(context);
    auto arrayBaseType = IntegerType::getInteger(8)->generateLlvmType(context);

    auto arrayPointerOffset = context->Builder->CreateStructGEP(llvmRecordType, V, 2, "string.ptr.offset");
    // const llvm::DataLayout &DL = context->TheModule->getDataLayout();
    // auto alignment = DL.getPrefTypeAlign(ptrType);
    auto loadResult =
            context->Builder->CreateLoad(llvm::PointerType::getUnqual(*context->TheContext), arrayPointerOffset);


    auto bounds =
            context->Builder->CreateGEP(arrayBaseType, loadResult, llvm::ArrayRef<llvm::Value *>{indexValue}, "", true);

    return context->Builder->CreateLoad(arrayBaseType, bounds);
}
