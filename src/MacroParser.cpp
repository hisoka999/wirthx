

#include "MacroParser.h"

#include <compare.h>
#include <exceptions/CompilerException.h>
#include <magic_enum/magic_enum.hpp>

MacroParser::MacroParser(const std::unordered_map<std::string, bool> &definitions) :
    m_current(0), m_definitions(definitions)
{
}

bool MacroParser::tryParseMacroDefinition(std::vector<Token> &result)
{
    if (!tryConsume(TokenType::MACRO_START))
    {
        return false;
    }
    consumeKeyWord("ifdef");
    Token macroKeyword = current();
    consume(TokenType::NAMEDTOKEN);
    bool useMacro = isVariableDefined(macroKeyword.lexical());

    consume(TokenType::MACRO_END);
    while (!canConsume(TokenType::MACRO_START))
    {

        if (useMacro && !canConsume(TokenType::MACRO_START) && !canConsume(TokenType::MACRO_END) &&
            !canConsume(TokenType::MACROKEYWORD))
        {
            result.push_back(current());
        }
        next();
    }
    consume(TokenType::MACRO_START);
    if (tryConsumeKeyWord("else"))
    {
        consume(TokenType::MACRO_END);
        while (!canConsume(TokenType::MACRO_START))
        {

            if (!useMacro && !canConsume(TokenType::MACRO_START) && !canConsume(TokenType::MACRO_END) &&
                !canConsume(TokenType::MACROKEYWORD))
            {
                result.push_back(current());
            }
            next();
        }
        consume(TokenType::MACRO_END);
        consume(TokenType::MACRO_START);
    }

    consumeKeyWord("endif");
    consume(TokenType::MACRO_END);

    return true;
}

bool MacroParser::hasError() const { return !m_errors.empty(); }
void MacroParser::printErrors(std::ostream &outputStream)
{
    for (auto &error: m_errors)
    {
        outputStream << error.token.sourceLocation.filename << ":" << error.token.row << ":" << error.token.col << ": "
                     << error.message << "\n";
    }
}


Token MacroParser::next()
{
    ++m_current;
    return current();
}
Token MacroParser::current() { return m_tokens[m_current]; }
bool MacroParser::isVariableDefined(const std::string &name) const { return m_definitions.contains(name); }
bool MacroParser::hasNext() const { return m_current < m_tokens.size() - 1; }
bool MacroParser::consume(const TokenType tokenType)
{
    if (canConsume(tokenType))
    {
        ++m_current;
        return true;
    }

    m_errors.push_back(ParserError{

            .token = m_tokens[m_current],
            .message = "expected token '" + std::string(magic_enum::enum_name(tokenType)) + "' but found " +
                       std::string(magic_enum::enum_name(m_tokens[m_current].tokenType)) + "!"});
    throw ParserException(m_errors);

    return false;
}
bool MacroParser::tryConsume(const TokenType tokenType)
{
    if (canConsume(tokenType))
    {
        next();
        return true;
    }
    return false;
}
bool MacroParser::canConsume(TokenType tokenType) const { return canConsume(tokenType, 0); }
bool MacroParser::canConsume(TokenType tokenType, size_t next) const
{
    return hasNext() && m_tokens[m_current + next].tokenType == tokenType;
}
bool MacroParser::canConsumeKeyWord(const std::string &keyword) const
{
    return canConsume(TokenType::MACROKEYWORD) && iequals(m_tokens[m_current].lexical(), keyword);
}

bool MacroParser::tryConsumeKeyWord(const std::string &keyword)
{
    if (canConsumeKeyWord(keyword))
    {
        next();
        return true;
    }
    return false;
}

bool MacroParser::consumeKeyWord(const std::string &keyword)
{
    if (tryConsumeKeyWord(keyword))
    {
        return true;
    }
    m_errors.push_back(ParserError{.token = m_tokens[m_current],
                                   .message = "expected keyword  '" + keyword + "' but found " +
                                              std::string(m_tokens[m_current].lexical()) + "!"});
    throw ParserException(m_errors);
}


std::vector<Token> MacroParser::parseFile(const std::vector<Token> &tokens)
{
    m_tokens = tokens;
    m_current = 0;
    std::vector<Token> result;


    while (m_current < tokens.size())
    {
        if (!tryParseMacroDefinition(result))
        {
            if (current().tokenType == TokenType::MACRO_END)
            {
                next();
            }
            result.push_back(current());
            if (hasNext())
                next();
            else
                break;
        }
    }

    return result;
}
