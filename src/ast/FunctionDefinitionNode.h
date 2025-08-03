#pragma once

#include <string>
#include <vector>
#include "ASTNode.h"
#include "BlockNode.h"

struct FunctionArgument
{
    std::shared_ptr<VariableType> type;
    std::string argumentName;
    Token token;
    bool isReference;
};

enum class FunctionAttribute
{
    Inline
};

enum class FunctionType
{
    Function,
    Procedure,
    Constructor,
    Destructor
};


class FunctionDefinitionNode : public ASTNode
{
private:
    std::string m_name;
    std::string m_externalName;
    std::string m_libName;
    std::vector<FunctionArgument> m_params;
    std::shared_ptr<BlockNode> m_body;
    FunctionType m_functionType;
    std::shared_ptr<VariableType> m_returnType;
    std::vector<FunctionAttribute> m_attributes;
    std::string m_functionSignature;
    std::optional<Token> m_parent;

public:
    FunctionDefinitionNode(const Token &token, std::string name, const std::vector<FunctionArgument> &params,
                           std::shared_ptr<BlockNode> body, FunctionType functionType,
                           std::optional<Token> parent = std::nullopt,
                           std::shared_ptr<VariableType> returnType = std::make_shared<VariableType>()

    );
    FunctionDefinitionNode(const Token &token, std::string name, std::string externalName, std::string libName,
                           const std::vector<FunctionArgument> &params, FunctionType functionType,
                           std::optional<Token> parent = std::nullopt,
                           std::shared_ptr<VariableType> returnType = std::make_shared<VariableType>());
    ~FunctionDefinitionNode() override = default;
    void print() override;
    std::string functionSignature();
    std::string &name();
    std::string &externalName();
    std::string &libName();
    std::shared_ptr<VariableType> returnType();
    [[nodiscard]] std::optional<FunctionArgument> getParam(const std::string &paramName) const;
    std::optional<FunctionArgument> getParam(const size_t index);
    [[nodiscard]] std::shared_ptr<BlockNode> body() const;
    llvm::Value *codegen(std::unique_ptr<Context> &context) override;

    void typeCheck(const std::unique_ptr<UnitNode> &unit, ASTNode *parentNode) override;
    void addAttribute(FunctionAttribute attribute);
    [[nodiscard]] FunctionType functionType() const;
    [[nodiscard]] std::optional<std::string> parent() const;
};
