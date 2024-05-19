#include "Lexer.h"
#include "Parser.h"
#include "ast/FunctionDefinitionNode.h"
#include "interpreter/Stack.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std::literals;

int main(int args, char **argv)
{
    bool displayAst = false;
    size_t fileArg = 1;
    if (args == 2)
    {
        if (argv[1] == "--version"sv || argv[1] == "-v"sv)
        {
            std::cout << "Version: 0.1\n";
            return 0;
        }
    }
    else if (argv[1] == "--ast"sv)
    {
        displayAst = true;
        fileArg++;
    }

    std::ifstream file;
    std::istringstream is;
    std::string s;
    std::string group;
    std::filesystem::path file_path(argv[fileArg]);
    if (!std::filesystem::exists(file_path))
    {
        std::cerr << "the first argument is not a valid input file\n";
        return 1;
    }

    file.open(file_path, std::ios::in);
    if (!file.is_open())
    {
        return 1;
    }
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    std::string buffer(size, ' ');
    file.seekg(0);
    file.read(&buffer[0], size);

    Lexer lexer;

    auto tokens = lexer.tokenize(std::string_view{buffer});
    // for (auto &token : tokens)
    // {
    //     std::cout << "token: " << token.lexical << " tokentype: " << static_cast<int>(token.tokenType) << "\n";
    // }
    Parser parser(file_path, tokens);
    auto asts = parser.parseTokens();
    std::vector<std::shared_ptr<ASTNode>> exec_nodes;
    if (parser.hasError())
    {
        parser.printErrors(std::cerr);
        return 1;
    }
    Stack stack;
    for (auto &ast : asts)
    {
        if (displayAst)
        {
            ast->print();
        }
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
        ast->eval(stack, std::cout);
    }
    return 0;
}