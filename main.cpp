#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include "Lexer.h"
#include "Parser.h"
#include "ast/FunctionDefinitionNode.h"
#include "compiler/Compiler.h"

using namespace std::literals;

enum class CompileOption
{
    COMPILE,
    JIT
};

std::string shiftarg(std::vector<std::string> &args)
{
    auto result = args.front();
    args.erase(args.begin());
    return result;
}

int main(int args, char **argv)
{
    // bool displayAst = false;
    // size_t fileArg = 1;
    std::vector<std::string> argList;
    for (int i = 0; i < args; i++)
    {
        argList.emplace_back(argv[i]);
    }
    auto compilerPath = shiftarg(argList);

    if (argList.size() == 1)
    {
        if (argList[0] == "--version"sv || argv[1] == "-v"sv)
        {
            std::cout << "Version: 0.1\n";
            return 0;
        }
    }
    CompileOption option = CompileOption::COMPILE;

    while (argList.size() > 1)
    {
        auto arg = shiftarg(argList);
        if (arg == "--ast"sv)
        {
            // displayAst = true;
            // fileArg++;
        }
        else if (arg == "-c")
        {
            option = CompileOption::COMPILE;
        }
    }

    std::ifstream file;
    std::istringstream is;
    std::string s;
    std::string group;
    std::filesystem::path file_path(argList[0]);
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

    switch (option)
    {
        case CompileOption::COMPILE:
            compile_file(file_path, std::cerr, std::cout);
            break;
        case CompileOption::JIT:
            assert(false && "JIT compiler is not implemented yet.");
    }
    return 0;
}
