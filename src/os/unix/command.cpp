#include "os/command.h"
#include <climits>
#include <iostream>
#include <llvm/Support/FileSystem.h>
bool execute_command_list(std::ostream &outstream, std::ostream &errorStream, const std::string &command,
                          std::vector<std::string> args)
{
    int LINE_LEN = 1024;
    char line[LINE_LEN];

    int pfd[2];
    if (pipe(pfd) < 0)
        return -1;
    auto perr = fdopen(pfd[0], "r");
    std::string cmd = command;

    for (auto &arg: args)
        cmd += " " + arg;
    cmd += " 2>&" + std::to_string(pfd[1]);

    int status = 0;
    if (auto pout = popen(cmd.c_str(), "r"))
    {
        close(pfd[1]);

        while (fgets(line, LINE_LEN, pout) != NULL)
            outstream << line;
        while (fgets(line, LINE_LEN, perr) != NULL)
            errorStream << line;

        status = pclose(pout);
    }

    fclose(perr);
    close(pfd[0]), close(pfd[1]);
    return status != -1;
}
