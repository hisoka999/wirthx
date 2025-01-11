#pragma once
#include <string_view>
#include <vector>
#include "Token.h"

inline std::vector<std::string> possible_tokens = {"program",   "unit",         "uses",
                                                   "begin",     "end",          "procedure",
                                                   "function",  "var",          "if",
                                                   "then",      "else",         "while",
                                                   "do",        "for",          "to",
                                                   "break",     "repeat",       "until",
                                                   "type",      "array",        "of",
                                                   "const",     "true",         "false",
                                                   "and",       "or",           "not",
                                                   "record",    "external",     "name",
                                                   "mod",       "inline",       "implementation",
                                                   "interface", "finalization", "initialization",
                                                   "div",       "downto"};

class Lexer
{
private:
    bool find_fixed_token(const std::string &content, size_t start, size_t *endPosition);
    bool find_token(const std::string &content, size_t start, size_t *endPosition);

    bool find_string(const std::string &content, size_t start, size_t *endPosition);
    bool find_number(const std::string &content, size_t start, size_t *endPosition);
    bool find_comment(const std::string &content, size_t start, size_t *endPosition);
    bool find_escape_sequence(const std::string &content, size_t start, size_t *endPosition);

public:
    Lexer();
    ~Lexer();

    std::vector<Token> tokenize(const std::string &filename, const std::string &content);
};
