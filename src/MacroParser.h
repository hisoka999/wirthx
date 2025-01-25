#pragma once
#include <unordered_map>
#include <vector>
#include "Token.h"

#include <exceptions/CompilerException.h>

typedef std::unordered_map<std::string, bool> MacroMap;

class MacroParser
{

private:
    std::vector<Token> m_tokens;
    size_t m_current;
    std::vector<ParserError> m_errors;
    MacroMap m_definitions;

    bool tryParseMacroDefinition(std::vector<Token> &result);
    bool hasError() const;
    void printErrors(std::ostream &outputStream);

    Token next();
    Token current();
    [[nodiscard]] bool isVariableDefined(const std::string &name) const;
    [[nodiscard]] bool hasNext() const;
    bool consume(TokenType tokenType);
    bool tryConsume(TokenType tokenType);
    [[nodiscard]] bool canConsume(TokenType tokenType) const;
    [[nodiscard]] bool canConsume(TokenType tokenType, size_t next) const;
    bool canConsumeKeyWord(const std::string &keyword, size_t next = 0) const;
    bool tryConsumeKeyWord(const std::string &keyword);
    bool consumeKeyWord(const std::string &keyword);

public:
    explicit MacroParser(const MacroMap &definitions);
    void parseIfDef(std::vector<Token> &result);
    ~MacroParser() = default;

    MacroMap macroDefinitions() const;

    [[nodiscard]] std::vector<Token> parseFile(const std::vector<Token> &tokens);
};
