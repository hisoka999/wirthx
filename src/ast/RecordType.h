#pragma once
#include "VariableDefinition.h"
#include "VariableType.h"

#include <cstddef>
#include <optional>
#include <vector>

class RecordType : public VariableType
{
private:
    std::vector<VariableDefinition> m_fields;
    llvm::Type *m_cachedType = nullptr;

public:
    VariableDefinition getField(size_t index);
    size_t size();
    void addField(VariableDefinition &field);
    std::optional<VariableDefinition> getFieldByName(const std::string &fieldName);
    int getFieldIndexByName(const std::string &name);

    RecordType(std::vector<VariableDefinition> fields, std::string typeName);
    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context) override;
};
