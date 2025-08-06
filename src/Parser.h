#pragma once
#include <filesystem>
#include <map>
#include <memory>
#include <vector>
#include "Lexer.h"
#include "ast/ASTNode.h"
#include "ast/UnitNode.h"
#include "ast/VariableDefinition.h"
#include "ast/types/VariableType.h"
#include "exceptions/CompilerException.h"

#include <unordered_map>

#include "ast/types/ArrayType.h"
#include "ast/types/ClassType.h"
#include "ast/types/TypeRegistry.h"


class EnumType;


struct Scope
{
    std::shared_ptr<ASTNode> rootNode;
    std::shared_ptr<ClassType> classType = nullptr;
    size_t id;
};


class Parser
{
    std::vector<std::filesystem::path> m_rtlDirectories;
    std::filesystem::path m_file_path;
    size_t m_current = 0;
    std::vector<Token> m_tokens;
    std::vector<ParserError> m_errors;
    TypeRegistry m_typeDefinitions;
    std::vector<VariableDefinition> m_known_variable_definitions;
    std::vector<std::string> m_known_function_names;
    std::vector<std::shared_ptr<FunctionDefinitionNode>> m_functionDeclarations;
    std::vector<std::shared_ptr<FunctionDefinitionNode>> m_functionDefinitions;
    std::vector<std::shared_ptr<ASTNode>> m_nodes;
    std::unordered_map<std::string, bool> m_definitions;
    bool m_includeSystem = false;

    Token next();
    Token current();
    [[nodiscard]] bool isConstantDefined(const std::string_view &name, const Scope &scope);

    [[nodiscard]] bool isVariableDefined(const std::string_view &name, const Scope &scope);
    [[nodiscard]] bool hasNext() const;
    bool consume(TokenType tokenType);
    bool tryConsume(TokenType tokenType);
    [[nodiscard]] bool canConsume(TokenType tokenType) const;
    [[nodiscard]] bool canConsume(TokenType tokenType, size_t next) const;
    bool consumeKeyWord(const std::string &keyword);
    bool tryConsumeKeyWord(const std::string &keyword);
    [[nodiscard]] bool canConsumeKeyWord(const std::string &keyword) const;
    [[nodiscard]] std::optional<std::shared_ptr<VariableType>>
    determinVariableTypeByName(const std::string &name) const;
    std::shared_ptr<ASTNode> parseEscapedString(const Token &token);
    std::shared_ptr<ASTNode> parseNumber();
    AccessModifier tryParseAccessModifier(AccessModifier defaultModifier = AccessModifier::Public);
    std::optional<std::shared_ptr<VariableType>> parseVariableType(const Scope &scope, bool includeErrors,
                                                                   const std::string &typeName = "");
    void parseTypeDefinitions(const Scope &scope);
    std::optional<VariableDefinition> parseConstantDefinition(const Scope &scope);
    std::shared_ptr<ASTNode> parseArrayConstructor(const Scope &scope);
    std::vector<VariableDefinition> parseVariableDefinitions(const Scope &scope);
    std::optional<std::shared_ptr<ArrayType>> parseArray(const Scope &scope);
    std::shared_ptr<ASTNode> parseStatement(const Scope &scope, bool withSemicolon = true);
    void parseConstantDefinitions(const Scope &scope, std::vector<VariableDefinition> &variable_definitions);
    std::shared_ptr<ASTNode> parseBaseExpression(const Scope &scope, const std::shared_ptr<ASTNode> &origLhs = nullptr,
                                                 bool includeCompare = true);
    std::shared_ptr<ASTNode> parseExpression(const Scope &scope, const std::shared_ptr<ASTNode> &origLhs = nullptr);
    std::shared_ptr<ASTNode> parseLogicalExpression(const Scope &scope, std::shared_ptr<ASTNode> lhs);

    std::shared_ptr<BlockNode> parseBlock(const Scope &scope);
    std::shared_ptr<ASTNode> parseKeyword(const Scope &scope, bool withSemicolon);
    std::shared_ptr<ASTNode> parseFunctionCall(const Scope &scope);
    std::shared_ptr<ASTNode> parseVariableAssignment(const Scope &scope);
    std::optional<std::shared_ptr<EnumType>> tryGetEnumTypeByValue(const std::string &enumKey) const;
    std::shared_ptr<ASTNode> parseConstantAccess(const Scope &scope);
    std::shared_ptr<ASTNode> parseMethodCall(const Scope &scope, const Token &variableNameToken,
                                             const Token &methodNameToken);
    [[nodiscard]] bool isFieldAMethodCall(const std::string &variableName, const std::string &methodName,
                                          const Scope &scope) const;
    [[nodiscard]] bool isClassInstance(const Token &token) const;
    std::shared_ptr<ASTNode> parseVariableAccess(const Scope &scope);
    std::shared_ptr<ASTNode> parseToken(const Scope &scope);
    std::shared_ptr<ASTNode> parseRangeElementOrType(const Scope &scope);
    std::shared_ptr<ASTNode> parseRangeElement(const Scope &scope);

    std::shared_ptr<FunctionDefinitionNode> parseFunctionDeclaration(const Scope &scope, FunctionType functionType);
    std::shared_ptr<FunctionDefinitionNode> parseFunctionDefinition(const Scope &scope, FunctionType functionType);

    std::unique_ptr<UnitNode> parseUnit(bool includeSystem);
    bool importUnit(const Token &token, const std::string &filename, bool includeSystem = true);

    bool isFunctionDeclared(const std::string &name) const;

    [[nodiscard]] std::unique_ptr<UnitNode> parseUnit();
    [[nodiscard]] std::unique_ptr<UnitNode> parseProgram();

    void parseInterfaceSection();
    void parseImplementationSection(bool includeSystem);
    void checkLhsExists(const std::shared_ptr<ASTNode> &lhs, const Token &token);

public:
    Parser(const std::vector<std::filesystem::path> &rtlDirectories, std::filesystem::path path,
           const std::unordered_map<std::string, bool> &definitions, const std::vector<Token> &tokens);
    ~Parser() = default;
    [[nodiscard]] bool hasError() const;
    [[nodiscard]] bool hasMessages() const;
    void printErrors(std::ostream &outputStream, bool printColor) const;

    [[nodiscard]] std::unique_ptr<UnitNode> parseFile();
    std::vector<ParserError> getErrors() { return m_errors; }
};
