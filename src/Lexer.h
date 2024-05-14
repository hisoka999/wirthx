#pragma once
#include <string_view>
#include <vector>

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

inline auto possible_tokens = {
    "program", "begin", "end", "procedure", "function", "var", "if", "then", "else"};

class Lexer
{
private:
    bool find_fixed_token(std::string_view content, size_t start, size_t *endPosition);
    bool find_token(std::string_view content, size_t start, size_t *endPosition);

    bool find_string(std::string_view content, size_t start, size_t *endPosition);
    bool find_number(std::string_view content, size_t start, size_t *endPosition);
    bool find_comment(std::string_view content, size_t start, size_t *endPosition);

public:
    Lexer();
    ~Lexer();

    std::vector<Token> tokenize(std::string_view content);
};
