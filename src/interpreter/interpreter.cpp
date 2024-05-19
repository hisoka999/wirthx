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
    auto asts = parser.parseTokens();
    std::vector<std::shared_ptr<ASTNode>> exec_nodes;
    if (parser.hasError())
    {
        parser.printErrors(errorStream);
        return;
    }
    Stack stack;
    for (auto &ast : asts)
    {

        auto func = std::dynamic_pointer_cast<FunctionDefinitionNode>(ast);
        if (func != nullptr)
        {
            stack.addFunction(func);
        }
        else
        {
            exec_nodes.push_back(ast);
        }
    }

    for (auto &ast : exec_nodes)
    {
        ast->eval(stack, outputStream);
    }
}