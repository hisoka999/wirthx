#pragma once
#include "Lexer.h"
#include "ast/ASTNode.h"
#include "ast/VariableType.h"
#include <filesystem>
#include <memory>
#include <vector>

struct ParserError
{
    std::string file_name;
    Token token;
    std::string message;
};

struct VariableDefinition
{
    VariableType variableType;
    std::string variableName;
    size_t scopeId;
};

class Parser
{
private:
    std::filesystem::path m_file_path;
    size_t m_current = 0;
    std::vector<Token> m_tokens;
    std::vector<ParserError> m_errors;
    std::vector<VariableDefinition> m_known_variable_definitions;
    std::vector<std::string> m_known_function_names;
    Token &next();
    Token &current();
    bool isVariableDefined(std::string_view name, size_t scope);
    bool hasNext();
    bool consume(TokenType tokenType);
    bool tryConsume(TokenType tokenType);
    bool canConsume(TokenType tokenType);
    bool consumeKeyWord(const std::string &keyword);
    bool tryConsumeKeyWord(const std::string &keyword);
    bool canConsumeKeyWord(const std::string &keyword);
    std::shared_ptr<ASTNode> parseToken(const Token &token, size_t currentScope, std::vector<std::shared_ptr<ASTNode>> nodes);
    bool parseKeyWord(const Token &currentToken, std::vector<std::shared_ptr<ASTNode>> &nodes, size_t scope);
    void parseFunction(size_t scope, std::vector<std::shared_ptr<ASTNode>> &nodes);
    void parseVariableAssignment(const Token &currentToken, size_t currentScope, std::vector<std::shared_ptr<ASTNode>> &nodes);
    std::shared_ptr<ASTNode> parseComparrision(const Token &currentToken, size_t currentScope, std::vector<std::shared_ptr<ASTNode>> &nodes);
    std::shared_ptr<ASTNode> parseExpression(const Token &currentToken, size_t currentScope);
    std::shared_ptr<ASTNode> parseBlock(const Token &currentToken, size_t scope);

public:
    Parser(const std::filesystem::path &path, std::vector<Token> &tokens);
    ~Parser();
    bool hasError() const;
    void printErrors(std::ostream &outputStream);

    std::vector<std::shared_ptr<ASTNode>> parseTokens();
};
