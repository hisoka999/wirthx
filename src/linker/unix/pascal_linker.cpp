#include "linker/pascal_linker.h"
#include "os/command.h"
#include <iostream>

bool pascal_link_modules(const std::filesystem::path &baseDir, const std::string &program_name, std::vector<std::string> flags, std::vector<std::string> object_files)
{
    std::vector<std::string> args;
    args.emplace_back("-nostdlib");
    args.emplace_back("-o");
    args.emplace_back((baseDir / program_name).string());
    for (auto &obj : object_files)
        args.emplace_back((baseDir / obj).string());

    for (auto &flag : flags)
        args.emplace_back(flag);

    return execute_command_list(std::cerr, "cc", args);
}
