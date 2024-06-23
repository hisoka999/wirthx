#include "interpreter.h"
#include "Lexer.h"
#include "Parser.h"
#include "ast/FunctionDefinitionNode.h"
#include "interpreter/Stack.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

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
    auto unit = parser.parseUnit();
    if (parser.hasError())
    {
        parser.printErrors(errorStream);
        return;
    }
    Stack stack;
    for (auto &func : unit->getFunctionDefinitions())
    {
        stack.addFunction(func);
    }

    unit->eval(stack, outputStream);
}