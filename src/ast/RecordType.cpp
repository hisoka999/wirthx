#include "RecordType.h"
#include "compiler/Context.h"

RecordType::RecordType(std::vector<VariableDefinition> fields, std::string typeName) : m_fields(std::move(fields))
{
    this->typeName = typeName;
    this->baseType = VariableBaseType::Struct;
}

void RecordType::addField(VariableDefinition &field) { m_fields.push_back(field); }

VariableDefinition RecordType::getField(size_t index) { return m_fields.at(index); }

std::optional<VariableDefinition> RecordType::getFieldByName(const std::string &fieldName)
{
    for (auto &field: m_fields)
    {
        if (field.variableName == fieldName)
        {
            return field;
        }
    }
    return std::nullopt;
}

int RecordType::getFieldIndexByName(const std::string &name)
{
    for (size_t i = 0; i < m_fields.size(); ++i)
    {
        if (m_fields[i].variableName == name)
        {
            return i;
        }
    }
    return 0;
}

llvm::Type *RecordType::generateLlvmType(std::unique_ptr<Context> &context)
{
    if (m_cachedType == nullptr)
    {
        std::vector<llvm::Type *> types;
        for (size_t i = 0; i < size(); ++i)
        {
            types.emplace_back(getField(i).variableType->generateLlvmType(context));
        }

        llvm::ArrayRef<llvm::Type *> Elements(types);


        m_cachedType = llvm::StructType::create(Elements, typeName);
    }
    return m_cachedType;
}

size_t RecordType::size() { return m_fields.size(); }
