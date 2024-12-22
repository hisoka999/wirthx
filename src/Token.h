#pragma once
#include <filesystem>
#include <string_view>

enum class TokenType : uint16_t
{
    NUMBER,
    STRING,
    ESCAPED_STRING,
    CHAR,
    KEYWORD,
    NAMEDTOKEN,
    PLUS,
    MINUS,
    DIV,
    MUL,
    LEFT_CURLY,
    RIGHT_CURLY,
    LEFT_SQUAR,
    RIGHT_SQUAR,
    EQUAL,
    GREATER,
    LESS,
    ENDLINE,
    T_EOF,
    COMMA,
    SEMICOLON,
    DOT,
    COLON
};

struct Token
{
    std::string_view lexical;
    size_t row;
    size_t col;
    TokenType tokenType;
    Token() = default;
    Token(const std::string_view lexical, const size_t row, const size_t col, const TokenType tokenType) :
        lexical(lexical), row(row), col(col), tokenType(tokenType)
    {
    }

    Token(const Token &other) : lexical(other.lexical), row(other.row), col(other.col), tokenType(other.tokenType) {}
    Token(Token &&other) noexcept : lexical(other.lexical), row(other.row), col(other.col), tokenType(other.tokenType)
    {
    }

    Token &operator=(const Token &other) = default;
};

struct TokenWithFile
{
    Token token;
    std::filesystem::path fileName;
};
