#pragma once

#include <optional>
#include <vector>


#include "ast/VariableDefinition.h"


class FunctionDefinitionNode;
enum class AccessModifier
{
    Public,
    Private,
    Protected,
    Published
};

enum class VirtualModifier
{
    None,
    Virtual,
    Override
};

struct ClassMember
{
    AccessModifier accessModifier;
    std::shared_ptr<VariableDefinition> variableDefinition;
    bool strict;
};

struct MemberFunction
{
    AccessModifier accessModifier;
    std::shared_ptr<FunctionDefinitionNode> functionDefinition;
    bool strict;
    VirtualModifier virtualModifier = VirtualModifier::None;
};


class ClassType final : public VariableType
{
    std::vector<ClassMember> m_members;
    std::vector<MemberFunction> m_memberFunctions;
    std::shared_ptr<FunctionDefinitionNode> m_constructor;
    std::shared_ptr<ClassType> m_baseClass;

public:
    explicit ClassType(const std::string &className) :
        VariableType(VariableBaseType::Class, className), m_baseClass(nullptr)
    {
    }
    ~ClassType() override = default;
    void addMember(const ClassMember &member) { m_members.push_back(member); }
    void addMemberFunction(const MemberFunction &memberFunction) { m_memberFunctions.push_back(memberFunction); }
    void setConstructor(const std::shared_ptr<FunctionDefinitionNode> &constructor) { m_constructor = constructor; }
    [[nodiscard]] std::shared_ptr<FunctionDefinitionNode> constructor() const { return m_constructor; }

    [[nodiscard]] const std::vector<ClassMember> &members() const { return m_members; }
    [[nodiscard]] const std::vector<MemberFunction> &memberFunctions() const { return m_memberFunctions; }
    std::optional<MemberFunction> getMemberFunction(const std::string &functionName);
    std::optional<ClassMember> member(const std::string &name);

    llvm::Type *generateLlvmType(std::unique_ptr<Context> &context) override;
    [[nodiscard]] size_t getFieldIndexByName(const std::string &name) const;
    [[nodiscard]] bool hasMemberFunction(const std::string &functionName) const;
    void setBaseClass(const std::shared_ptr<ClassType> &value);
    [[nodiscard]] std::shared_ptr<ClassType> baseClass() const;

    [[nodiscard]] llvm::Value *generateParentAccess(llvm::Value *objectValue, std::unique_ptr<Context> &context);
};
