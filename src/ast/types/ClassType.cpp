#include "ClassType.h"

#include <llvm/IR/DerivedTypes.h>

#include "ast/FunctionDefinitionNode.h"
#include "compare.h"
#include "compiler/Context.h"
#include "exceptions/CompilerException.h"

std::optional<MemberFunction> ClassType::getMemberFunction(const std::string &functionName)
{
    for (auto &memberFunction: m_memberFunctions)
    {
        if (iequals(memberFunction.functionDefinition->name(), functionName))
        {
            return memberFunction;
        }
    }
    return std::nullopt;
}
std::optional<ClassMember> ClassType::member(const std::string &name)
{
    for (auto &member: m_members)
    {
        if (iequals(member.variableDefinition->variableName, name))
        {
            return member;
        }
    }
    return std::nullopt;
}
llvm::Type *ClassType::generateLlvmType(std::unique_ptr<Context> &context)
{
    const auto cached_type = llvm::StructType::getTypeByName(*context->context(), typeName);
    if (cached_type == nullptr)
    {
        std::vector<llvm::Type *> types;
        for (const auto &member: m_members)
        {
            types.emplace_back(member.variableDefinition->variableType->generateLlvmType(context));
        }

        const llvm::ArrayRef<llvm::Type *> elements(types);


        return llvm::StructType::create(*context->context(), elements, typeName);
    }
    return cached_type;
}
size_t ClassType::getFieldIndexByName(const std::string &name) const
{
    size_t index = 0;
    for (auto &member: m_members)
    {
        if (iequals(member.variableDefinition->variableName, name))
        {
            return index;
        }
        index++;
    }
    throw std::runtime_error("Cannot find field with name: " + name + " in class: " + typeName);
}
bool ClassType::hasMemberFunction(const std::string &functionName) const
{
    return std::ranges::any_of(m_memberFunctions, [&functionName](const MemberFunction &memberFunction)
                               { return iequals(memberFunction.functionDefinition->name(), functionName); });
}
