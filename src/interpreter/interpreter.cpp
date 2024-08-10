#include "interpreter.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "ast/FunctionDefinitionNode.h"
#include "interpreter/InterpreterContext.h"
#include "interpreter/Stack.h"

void interprete_file(std::filesystem::path inputPath, std::ostream &errorStream, std::ostream &outputStream)
{
    std::ifstream file;
    std::istringstream is;

    file.open(inputPath, std::ios::in);
    if (!file.is_open())
    {
        return;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);

    Lexer lexer;

    auto tokens = lexer.tokenize(std::string_view{buffer});

    Parser parser(inputPath, tokens);
    InterpreterContext context;
    context.unit = parser.parseUnit();
    if (parser.hasError())
    {
        parser.printErrors(errorStream);
        return;
    }
    try
    {
        for (auto &func: context.unit->getFunctionDefinitions())
        {
            context.stack.addFunction(func);
        }

        context.unit->eval(context, outputStream);
    }
    catch (CompilerException &e)
    {
        errorStream << e.what();
    }
}
