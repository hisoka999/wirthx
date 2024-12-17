#include "os/command.h"
#include <climits>
#include <cstdlib>
#include <iostream>

bool execute_command_list(std::ostream &outstream, const std::string &command, std::vector<std::string> args)
{
    char path[PATH_MAX];
    std::string cmd = command;

    for (auto &arg: args)
        cmd += " " + arg;

    const auto fp = popen(cmd.c_str(), "r");
    if (fp == nullptr)
        /* Handle error */;

    while (fgets(path, PATH_MAX, fp) != nullptr)
        outstream << path;

    const int status = pclose(fp);
    if (status != 0)
    {
        std::cerr << "could not execute command: " << cmd << "\n";
    }

    return status != -1;
}
