#include <iostream>
#include <cstring>
#include <fstream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "interpreter/Stack.h"
#include "ast/FunctionDefinitionNode.h"
#include <filesystem>

using namespace std::literals;

int main(int args, char **argv)
{
    if (args == 2)
    {
        if (argv[1] == "--version"sv || argv[1] == "-v"sv)
        {
            std::cout << "Version: 0.1\n";
            return 0;
        }
    }

    std::ifstream file;
    std::istringstream is;
    std::string s;
    std::string group;
    std::filesystem::path file_path(argv[1]);
    if (!std::filesystem::exists(file_path))
    {
        std::cerr << "the first argument is not a valid input file\n";
        return 1;
    }

    file.open(argv[1], std::ios::in);
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
        parser.printErrors();
        return 1;
    }
    Stack stack;
    for (auto &ast : asts)
    {
        ast->print();
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
        ast->eval(stack);
    }
    return 0;
}