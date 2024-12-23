#include "Lexer.h"
#include <iostream>
#include "compare.h"

Lexer::Lexer() {}

Lexer::~Lexer() {}

bool validStartNameChar(char value)
{
    return (value >= 'A' && value <= 'Z') || (value >= 'a' && value <= 'z') || value == '_';
}

bool validNameChar(char value) { return validStartNameChar(value) || (value >= '0' && value <= '9'); }

std::vector<Token> Lexer::tokenize(std::string_view content)
{
    std::vector<Token> tokens;

    size_t row = 1;
    size_t column = 1;
    for (size_t i = 0; i < content.size(); ++i)
    {

        auto ch = content[i];
        size_t endPosition = i;
        bool found = find_comment(content, i, &endPosition);
        if (found)
        {
            // count lines
            for (size_t start = i; start < endPosition; start++)
            {
                if (content[start] == '\n')
                    row++;
            }
            int offset = endPosition - i;
            i = endPosition;
            column += offset + 1;
            continue;
        }

        found = find_fixed_token(content, i, &endPosition);
        if (found)
        {
            int offset = endPosition - i;
            tokens.push_back(Token(content.substr(i, offset), row, column, TokenType::KEYWORD));
            i = endPosition - 1;
            column += offset + 1;
            continue;
        }

        found = find_token(content, i, &endPosition);
        if (found)
        {
            int offset = endPosition - i;
            tokens.push_back(Token(content.substr(i, offset + 1), row, column, TokenType::NAMEDTOKEN));
            i = endPosition;
            column += offset + 1;
            continue;
        }

        endPosition = i;
        found = find_string(content, i, &endPosition);
        if (found)
        {
            int offset = endPosition - i;
            auto string_length = (offset - 1);
            if (string_length != 1)
                tokens.push_back(Token(content.substr(i + 1, string_length), row, column, TokenType::STRING));
            else
                tokens.push_back(Token(content.substr(i + 1, string_length), row, column, TokenType::CHAR));
            i = endPosition;
            column += offset + 1;
            continue;
        }

        found = find_escape_sequence(content, i, &endPosition);
        if (found)
        {
            int offset = endPosition - i;
            auto string_length = (offset);
            auto string = content.substr(i, string_length);


            if (string_length != 1)
                tokens.push_back(Token(string, row, column, TokenType::ESCAPED_STRING));
            else
                tokens.push_back(Token(string, row, column, TokenType::CHAR));
            i = endPosition - 1;
            column += offset;
            continue;
        }

        endPosition = i;
        found = find_number(content, i, &endPosition);
        if (found)
        {
            int offset = endPosition - i;
            tokens.push_back(Token(content.substr(i, offset + 1), row, column, TokenType::NUMBER));
            i = endPosition;
            column += offset + 1;
            continue;
        }
        // TokenType tokenType;
        switch (ch)
        {
            case '\n':
                // tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::ENDLINE));
                column = 1;
                row++;
                continue;
            case '+':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::PLUS));
                break;
            case '-':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::MINUS));
                break;
            case '*':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::MUL));
                break;
            case '/':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::DIV));
                break;
            case '(':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::LEFT_CURLY));
                break;
            case ')':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::RIGHT_CURLY));
                break;
            case '[':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::LEFT_SQUAR));
                break;
            case ']':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::RIGHT_SQUAR));
                break;
            case '=':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::EQUAL));
                break;
            case '<':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::LESS));
                break;
            case '>':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::GREATER));
                break;
            case ',':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::COMMA));
                break;
            case ';':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::SEMICOLON));
                break;
            case ':':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::COLON));
                break;
            case '.':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::DOT));
                break;
            case '^':
                tokens.push_back(Token(content.substr(i, 1), row, column, TokenType::CARET));
                break;
            default:
                break;
        }
        column++;
    }
    tokens.push_back(Token("EOF", row, column, TokenType::T_EOF));
    return tokens;
}

bool Lexer::find_escape_sequence(std::string_view content, size_t start, size_t *endPosition)
{
    char current = content[start];
    if (current != '#')
        return false;

    *endPosition = start + 1;
    current = content[start + 1];
    while (true)
    {
        if (current == '#')
        {
            *endPosition += 1;
            current = content[*endPosition];
        }
        else if (content[*endPosition] >= '0' && content[*endPosition] <= '9')
        {
            *endPosition += 1;
            current = content[*endPosition];
            continue;
        }
        else
        {
            break;
        }
    }
    return true;
}

bool Lexer::find_string(std::string_view content, size_t start, size_t *endPosition)
{
    char current = content[start];
    if (current != '\'')
        return false;
    *endPosition = start + 1;
    current = content[start + 1];
    while (true)
    {
        if (current == '\'')
        {
            if (content.size() - 1 > *endPosition + 1 && content[*endPosition + 1] == '\'')
            {
                *endPosition += 2;
                current = content[*endPosition];
            }
            else
            {
                break;
            }
        }

        *endPosition += 1;
        current = content[*endPosition];
    }
    return true;
}
bool isNumberStart(char c) { return (c >= '0' && c <= '9') || c == '-'; }

bool Lexer::find_number(std::string_view content, size_t start, size_t *endPosition)
{
    int index = 0;
    char current = content[start];
    if (current == '-' && !isNumberStart(content[start + 1]))
        return false;
    if (!isNumberStart(current))
        return false;
    while ((current >= '0' && current <= '9') || (index == 0 && current == '-'))
    {

        *endPosition += 1;
        current = content[*endPosition];
        index++;
    }
    if (current < '0' || current > '9')
        *endPosition -= 1;
    return true;
}

bool Lexer::find_token(std::string_view content, size_t start, size_t *endPosition)
{
    char current = content[start];
    *endPosition = start;
    if (!validStartNameChar(current))
        return false;

    while (validNameChar(current))
    {

        *endPosition += 1;
        current = content[*endPosition];
    }
    while (!validNameChar(current))
    {
        *endPosition -= 1;
        current = content[*endPosition];
    }
    return true;
}

bool Lexer::find_fixed_token(std::string_view content, size_t start, size_t *endPosition)
{

    char current = content[start];
    *endPosition = start + 1;
    if (!validStartNameChar(current))
        return false;

    while (validNameChar(current))
    {

        *endPosition += 1;
        current = content[*endPosition];
    }

    auto tmp = content.substr(start, *endPosition - start);
    for (auto token: possible_tokens)
    {
        if (iequals(tmp, token))
        {
            return true;
        }
    }

    return false;
}

bool Lexer::find_comment(std::string_view content, size_t start, size_t *endPosition)
{
    if (content[start] == '{')
    {
        char current = content[start];
        *endPosition = start + 1;

        while (current != '}' && current != 0)
        {
            *endPosition += 1;
            current = content[*endPosition];
        }
        *endPosition -= 1;
        return true;
    }
    if (content[start] == '/' && content[start + 1] == '/')
    {
        char current = content[start];
        *endPosition = start + 3;

        while (current != '\n' && current != 0)
        {
            *endPosition += 1;
            current = content[*endPosition];
        }
        *endPosition -= 1;
        return true;
    }

    return false;
}
