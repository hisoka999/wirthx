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

std::vector<Token> Lexer::tokenize(const std::string &filename, const std::string &content)
{
    std::vector<Token> tokens;

    size_t row = 1;
    size_t column = 1;
    auto contentPtr = std::make_shared<std::string>(content);
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
            const size_t offset = endPosition - i;
            SourceLocation source_location = {
                    .filename = filename, .source = contentPtr, .byte_offset = i, .num_bytes = offset};
            tokens.emplace_back(source_location, row, column, TokenType::KEYWORD);
            i = endPosition - 1;
            column += offset + 1;
            continue;
        }

        found = find_token(content, i, &endPosition);
        if (found)
        {
            const size_t offset = endPosition - i + 1;
            SourceLocation source_location = {
                    .filename = filename, .source = contentPtr, .byte_offset = i, .num_bytes = offset};
            tokens.emplace_back(source_location, row, column, TokenType::NAMEDTOKEN);
            i = endPosition;
            column += offset;
            continue;
        }

        endPosition = i;
        found = find_string(content, i, &endPosition);
        if (found)
        {
            size_t offset = endPosition - i;
            auto string_length = (offset - 1);
            SourceLocation source_location = {
                    .filename = filename, .source = contentPtr, .byte_offset = i + 1, .num_bytes = string_length};
            if (string_length != 1)
                tokens.emplace_back(source_location, row, column, TokenType::STRING);
            else
                tokens.emplace_back(source_location, row, column, TokenType::CHAR);
            i = endPosition;
            column += offset + 1;
            continue;
        }

        found = find_escape_sequence(content, i, &endPosition);
        if (found)
        {
            size_t offset = endPosition - i;
            auto string_length = (offset);

            SourceLocation source_location = {
                    .filename = filename, .source = contentPtr, .byte_offset = i, .num_bytes = string_length};
            if (string_length != 1)
                tokens.push_back(Token(source_location, row, column, TokenType::ESCAPED_STRING));
            else
                tokens.push_back(Token(source_location, row, column, TokenType::CHAR));
            i = endPosition - 1;
            column += offset;
            continue;
        }

        endPosition = i;
        found = find_number(content, i, &endPosition);
        if (found)
        {
            const size_t offset = endPosition - i + 1;
            SourceLocation source_location = {
                    .filename = filename, .source = contentPtr, .byte_offset = i, .num_bytes = offset};
            tokens.push_back(Token(source_location, row, column, TokenType::NUMBER));
            i = endPosition;
            column += offset;
            continue;
        }
        // TokenType tokenType;
        SourceLocation source_location = {.filename = filename, .source = contentPtr, .byte_offset = i, .num_bytes = 1};
        switch (ch)
        {
            case '\n':
                // tokens.push_back(Token(source_location, row, column, TokenType::ENDLINE));
                column = 1;
                row++;
                continue;
            case '+':
                tokens.emplace_back(source_location, row, column, TokenType::PLUS);
                break;
            case '-':
                tokens.emplace_back(source_location, row, column, TokenType::MINUS);
                break;
            case '*':
                tokens.emplace_back(source_location, row, column, TokenType::MUL);
                break;
            case '/':
                tokens.emplace_back(source_location, row, column, TokenType::DIV);
                break;
            case '(':
                tokens.emplace_back(source_location, row, column, TokenType::LEFT_CURLY);
                break;
            case ')':
                tokens.emplace_back(source_location, row, column, TokenType::RIGHT_CURLY);
                break;
            case '[':
                tokens.emplace_back(source_location, row, column, TokenType::LEFT_SQUAR);
                break;
            case ']':
                tokens.emplace_back(source_location, row, column, TokenType::RIGHT_SQUAR);
                break;
            case '=':
                tokens.emplace_back(source_location, row, column, TokenType::EQUAL);
                break;
            case '<':
                tokens.emplace_back(source_location, row, column, TokenType::LESS);
                break;
            case '>':
                tokens.emplace_back(source_location, row, column, TokenType::GREATER);
                break;
            case ',':
                tokens.emplace_back(source_location, row, column, TokenType::COMMA);
                break;
            case ';':
                tokens.emplace_back(source_location, row, column, TokenType::SEMICOLON);
                break;
            case ':':
                tokens.emplace_back(source_location, row, column, TokenType::COLON);
                break;
            case '.':
                tokens.emplace_back(source_location, row, column, TokenType::DOT);
                break;
            case '^':
                tokens.emplace_back(source_location, row, column, TokenType::CARET);
                break;
            case '!':
                tokens.emplace_back(source_location, row, column, TokenType::BANG);
                break;
            case '@':
                tokens.emplace_back(source_location, row, column, TokenType::AT);
                break;
            default:
                break;
        }
        column++;
    }
    SourceLocation source_location = {
            .filename = filename, .source = contentPtr, .byte_offset = content.size(), .num_bytes = 0};
    tokens.push_back(Token(source_location, row, column, TokenType::T_EOF));
    return tokens;
}

bool Lexer::find_escape_sequence(const std::string &content, size_t start, size_t *endPosition)
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

bool Lexer::find_string(const std::string &content, size_t start, size_t *endPosition)
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

bool Lexer::find_number(const std::string &content, size_t start, size_t *endPosition)
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

bool Lexer::find_token(const std::string &content, size_t start, size_t *endPosition)
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

bool Lexer::find_fixed_token(const std::string &content, size_t start, size_t *endPosition)
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

bool Lexer::find_comment(const std::string &content, size_t start, size_t *endPosition)
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
