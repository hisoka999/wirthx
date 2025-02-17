#include "FileType.h"

#include <llvm/IR/DerivedTypes.h>
#include <vector>

FileType::FileType(const std::string &typeName, std::optional<std::shared_ptr<VariableType>> childType) :
    VariableType(VariableBaseType::File, typeName), m_childType(std::move(childType))
{
}
llvm::Type *FileType::generateLlvmType(std::unique_ptr<Context> &context)
{
    if (m_cachedType == nullptr)
    {
        std::vector<llvm::Type *> types;
        types.emplace_back(::PointerType::getPointerTo(VariableType::getInteger(8))->generateLlvmType(context));
        types.emplace_back(VariableType::getPointer()->generateLlvmType(context));
        types.emplace_back(VariableType::getBoolean()->generateLlvmType(context));

        const llvm::ArrayRef<llvm::Type *> elements(types);


        m_cachedType = llvm::StructType::create(elements, typeName);
    }
    return m_cachedType;
}
std::shared_ptr<VariableType> FileType::getFileType(std::optional<std::shared_ptr<VariableType>> childType)
{
    static auto type = std::make_shared<FileType>("file", childType);
    return type;
}
