#include "os/command.h"
#include "limits.h"
#include <cstdlib>
#include <iostream>

bool execute_command_list(std::ostream &outstream, const std::string &command, std::vector<std::string> args)
{
    FILE *fp;
    int status;
    char path[PATH_MAX];

    std::string cmd = command;

    for (auto &arg : args)
        cmd += " " + arg;

    fp = popen(cmd.c_str(), "r");
    if (fp == NULL)
        /* Handle error */;

    while (fgets(path, PATH_MAX, fp) != NULL)
        outstream << path;

    status = pclose(fp);
    return status != -1;
}