#pragma once
#include <exception>
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
private:
    std::vector<std::filesystem::path> m_rtlDirectories;
    std::filesystem::path m_file_path;
    size_t m_current = 0;
    std::vector<Token> m_tokens;
    std::vector<ParserError> m_errors;
    std::map<std::string, std::shared_ptr<VariableType>> m_typeDefinitions;
    std::vector<VariableDefinition> m_known_variable_definitions;
    std::vector<std::string> m_known_function_names;
    Token &next();
    Token &current();
    bool isVariableDefined(std::string_view name, size_t scope);
    bool hasNext();
    bool consume(TokenType tokenType);
    bool tryConsume(TokenType tokenType);
    bool canConsume(TokenType tokenType);
    bool canConsume(TokenType tokenType, size_t next);
    bool consumeKeyWord(const std::string &keyword);
    bool tryConsumeKeyWord(const std::string &keyword);
    bool canConsumeKeyWord(const std::string &keyword);
    std::optional<std::shared_ptr<VariableType>> determinVariableTypeByName(const std::string &name);
    std::shared_ptr<ASTNode> parseToken(const Token &token, size_t currentScope,
                                        std::vector<std::shared_ptr<ASTNode>> nodes);
    const std::shared_ptr<ASTNode> parseNumber(const Token &token, size_t currentScope);
    std::shared_ptr<ArrayType> parseArray(size_t scope, bool includeExpressionEnd = true);
    bool parseKeyWord(const Token &currentToken, std::vector<std::shared_ptr<ASTNode>> &nodes, size_t scope);
    void parseFunction(size_t scope, std::vector<std::shared_ptr<ASTNode>> &nodes, bool isProcedure);
    void parseVariableAssignment(const Token &currentToken, size_t currentScope,
                                 std::vector<std::shared_ptr<ASTNode>> &nodes);
    std::optional<VariableDefinition> parseVariableDefinitions(const Token &token, size_t scope);
    std::shared_ptr<ASTNode> parseComparrision(const Token &currentToken, size_t currentScope,
                                               std::vector<std::shared_ptr<ASTNode>> &nodes);
    std::shared_ptr<ASTNode> parseExpression(const Token &currentToken, size_t currentScope);
    std::shared_ptr<BlockNode> parseBlock(const Token &currentToken, size_t scope);

public:
    Parser(const std::vector<std::filesystem::path> rtlDirectories, const std::filesystem::path &path,
           std::vector<Token> &tokens);
    ~Parser();
    bool hasError() const;
    void printErrors(std::ostream &outputStream);

    std::unique_ptr<UnitNode> parseUnit();
    void parseTypeDefinitions(int scope);
};
