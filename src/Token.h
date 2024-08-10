#pragma once
#include <filesystem>
#include <string_view>

enum class TokenType
{
    NUMBER,
    STRING,
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
};

struct TokenWithFile
{
    Token token;
    std::filesystem::path fileName;
};
