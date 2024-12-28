#pragma once
#include <filesystem>
#include <string_view>
#include <utility>
#include "SourceLocation.h"
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
    COLON,
    CARET,
    BANG
};


struct Token
{
    SourceLocation sourceLocation;
    size_t row{};
    size_t col{};
    TokenType tokenType;

    Token() : sourceLocation(), tokenType(TokenType::T_EOF) {}

    Token(SourceLocation sourceLocation, const size_t row, const size_t col, const TokenType tokenType) :
        sourceLocation(std::move(sourceLocation)), row(row), col(col), tokenType(tokenType)
    {
    }

    Token(const Token &other) = default;
    Token(Token &&other) noexcept :
        sourceLocation(other.sourceLocation), row(other.row), col(other.col), tokenType(other.tokenType)
    {
    }

    Token &operator=(const Token &other) = default;

    [[nodiscard]] std::string lexical() const { return sourceLocation.text(); }
};
