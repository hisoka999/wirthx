#include "CompilerException.h"

#include <cassert>

std::string outputTypeString(OutputType outputType)
{
    switch (outputType)
    {
        case OutputType::ERROR:
            return "error";
        case OutputType::WARN:
            return "warn";
        case OutputType::HINT:
            return "info";
    }
    assert(false && "Unknown output type");
    return "unknown";
}
Color::Modifier outputTypeToColor(OutputType outputType)
{
    switch (outputType)
    {
        case OutputType::ERROR:
            return Color::FG_RED;
        case OutputType::WARN:
            return Color::FG_GREEN;
        case OutputType::HINT:
            return Color::FG_BLUE;
    }
    assert(false && "Unknown output type");
    return Color::FG_DEFAULT;
}
void ParserError::msg(std::ostream &ostream, bool printColor) const
{
    if (printColor)
        ostream << token.sourceLocation.filename << ":" << token.row << ":" << token.col << ": "
                << outputTypeToColor(outputType) << outputTypeString(outputType) << Color::Modifier(Color::FG_DEFAULT)
                << ": " << message << "\n";
    else
        ostream << token.sourceLocation.filename << ":" << token.row << ":" << token.col << ": "
                << outputTypeString(outputType) << ": " << message << "\n";
}
CompilerException::CompilerException(ParserError error)
{
    std::stringstream outputStream;
    error.msg(outputStream, false);

    m_message = outputStream.str();
}
CompilerException::CompilerException(std::vector<ParserError> errors)
{
    std::stringstream outputStream;
    for (auto &error: errors)
    {
        error.msg(outputStream, false);
    }
    m_message = outputStream.str();
}
