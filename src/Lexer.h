#pragma once
#include <string_view>
#include <vector>
#include "Token.h"


class Lexer
{
private:
    bool find_fixed_token(const std::string &content, size_t start, size_t *endPosition);
    bool find_token(const std::string &content, size_t start, size_t *endPosition);

    bool find_string(const std::string &content, size_t start, size_t *endPosition);
    bool find_number(const std::string &content, size_t start, size_t *endPosition);
    bool find_comment(const std::string &content, size_t start, size_t *endPosition);
    bool find_escape_sequence(const std::string &content, size_t start, size_t *endPosition);
    bool find_macro_keyword(const std::string &content, size_t start, size_t *endPosition);

public:
    Lexer();
    ~Lexer();

    std::vector<Token> tokenize(const std::string &filename, const std::string &content);
};
