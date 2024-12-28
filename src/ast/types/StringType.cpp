#include "StringType.h"
#include <cassert>
#include <llvm/IR/IRBuilder.h>

#include "compiler/Context.h"
#include "exceptions/CompilerException.h"


llvm::Type *StringType::generateLlvmType(std::unique_ptr<Context> &context)
{
    if (llvmType == nullptr)
    {
        const auto baseType = IntegerType::getInteger(8);
        const auto charType = baseType->generateLlvmType(context);
        std::vector<llvm::Type *> types;
        types.emplace_back(VariableType::getInteger(64)->generateLlvmType(context));
        types.emplace_back(VariableType::getInteger(64)->generateLlvmType(context));
        types.emplace_back(llvm::PointerType::getUnqual(charType));


        llvm::ArrayRef<llvm::Type *> Elements(types);


        llvmType = llvm::StructType::create(Elements, "string");
    }
    return llvmType;
}

llvm::Value *StringType::generateFieldAccess(Token &token, llvm::Value *indexValue, std::unique_ptr<Context> &context)
{
    auto arrayName = std::string(token.lexical());
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

std::shared_ptr<StringType> StringType::getString()
{
    static auto stringType = std::make_shared<StringType>();
    stringType->baseType = VariableBaseType::String;
    stringType->typeName = "string";
    return stringType;
}
