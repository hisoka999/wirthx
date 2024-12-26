#pragma once
#include <filesystem>
#include <map>
#include <memory>
#include <vector>
#include "Lexer.h"
#include "ast/ASTNode.h"
#include "ast/UnitNode.h"
#include "ast/VariableDefinition.h"
#include "ast/VariableType.h"
#include "exceptions/CompilerException.h"


using ParserException = CompilerException;
class Parser
{
    std::vector<std::filesystem::path> m_rtlDirectories;
    std::filesystem::path m_file_path;
    size_t m_current = 0;
    std::vector<Token> m_tokens;
    std::vector<ParserError> m_errors;
    std::map<std::string, std::shared_ptr<VariableType>> m_typeDefinitions;
    std::vector<VariableDefinition> m_known_variable_definitions;
    std::vector<std::string> m_known_function_names;
    std::vector<std::shared_ptr<FunctionDefinitionNode>> m_functionDefinitions;
    std::vector<std::shared_ptr<ASTNode>> m_nodes;


    Token next();
    Token current();
    [[nodiscard]] bool isVariableDefined(std::string_view name, size_t scope);
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
    void parseTypeDefinitions(size_t scope);
    std::optional<VariableDefinition> parseConstantDefinition(size_t scope);
    std::vector<VariableDefinition> parseVariableDefinitions(size_t scope);
    std::shared_ptr<ArrayType> parseArray(size_t scope);
    std::shared_ptr<ASTNode> parseStatement(size_t scope);
    std::shared_ptr<ASTNode> parseBaseExpression(size_t scope, const std::shared_ptr<ASTNode> &origLhs = nullptr);
    std::shared_ptr<ASTNode> parseExpression(size_t scope, const std::shared_ptr<ASTNode> &origLhs = nullptr);
    std::shared_ptr<ASTNode> parseLogicalExpression(size_t scope, std::shared_ptr<ASTNode> lhs);

    std::shared_ptr<BlockNode> parseBlock(size_t scope);
    std::shared_ptr<ASTNode> parseKeyword(size_t scope);
    std::shared_ptr<ASTNode> parseFunctionCall(size_t scope);
    std::shared_ptr<ASTNode> parseVariableAssignment(size_t scope);
    std::shared_ptr<ASTNode> parseVariableAccess(size_t scope);
    std::shared_ptr<ASTNode> parseToken(size_t scope);
    std::shared_ptr<FunctionDefinitionNode> parseFunctionDefinition(size_t scope, bool isFunction);

    bool importUnit(const std::string &filename);

public:
    Parser(const std::vector<std::filesystem::path> &rtlDirectories, std::filesystem::path path,
           const std::vector<Token> &tokens);
    ~Parser() = default;
    [[nodiscard]] bool hasError() const;
    void printErrors(std::ostream &outputStream);

    [[nodiscard]] std::unique_ptr<UnitNode> parseUnit();
};
