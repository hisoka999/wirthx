
#pragma once
#include <exception>
#include <sstream>
#include <string>
#include <vector>
#include "Token.h"

struct ParserError
{
    std::string file_name;
    Token token;
    std::string message;
};

class CompilerException : public std::exception
{
private:
    std::string mesasage;

public:
    CompilerException(ParserError error)
    {
        std::stringstream outputStream;

        outputStream << error.file_name << ":" << error.token.row << ":" << error.token.col << ": " << error.message
                     << "\n";

        mesasage = outputStream.str();
    }
    CompilerException(std::vector<ParserError> errors)
    {
        std::stringstream outputStream;
        for (auto &error: errors)
        {
            outputStream << error.file_name << ":" << error.token.row << ":" << error.token.col << ": " << error.message
                         << "\n";
        }
        mesasage = outputStream.str();
    }

    const char *what() const noexcept override { return mesasage.c_str(); }
};
