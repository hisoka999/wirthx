#pragma once

#include <filesystem>
#include <string>
#include <vector>

enum class CompileOption
{
    COMPILE,
    JIT
};
enum class BuildMode
{
    Debug,
    Release
};

struct CompilerOptions
{
    CompileOption option = CompileOption::COMPILE;
    BuildMode buildMode = BuildMode::Debug;

    std::filesystem::path outputDirectory;
    std::vector<std::filesystem::path> rtlDirectories;
    std::string compilerPath;
    bool runProgram = false;
    bool printLLVMIR = false;
    bool printAST = false;
    bool lsp = false;
};

std::string shiftarg(std::vector<std::string> &args);

CompilerOptions parseCompilerOptions(std::vector<std::string> &argList);
