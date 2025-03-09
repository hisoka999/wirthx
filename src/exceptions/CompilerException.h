
#pragma once
#include <exception>
#include <sstream>
#include <string>
#include <vector>
#include "Token.h"

enum class OutputType
{
    ERROR,
    WARN,
    HINT
};

namespace Color
{
    enum Code
    {
        FG_RED = 31,
        FG_GREEN = 32,
        FG_BLUE = 34,
        FG_DEFAULT = 39,
        BG_RED = 41,
        BG_GREEN = 42,
        BG_BLUE = 44,
        BG_DEFAULT = 49
    };
    class Modifier
    {
        Code code;

    public:
        Modifier(Code pCode) : code(pCode) {}
        friend std::ostream &operator<<(std::ostream &os, const Modifier &mod)
        {
            return os << "\033[" << mod.code << "m";
        }
    };
} // namespace Color

struct ParserError
{
    OutputType outputType = OutputType::ERROR;
    Token token;
    std::string message;

    void msg(std::ostream &ostream, bool printColor) const;
};

class CompilerException : public std::exception
{
private:
    std::string m_message;

public:
    explicit CompilerException(ParserError error);
    explicit CompilerException(std::vector<ParserError> errors);

    [[nodiscard]] const char *what() const noexcept override { return m_message.c_str(); }
};


using ParserException = CompilerException;
