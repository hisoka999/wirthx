#pragma once

#include <ostream>
#include <string>
#include <vector>

bool execute_command_list(std::ostream &outstream, std::ostream &errorStream, const std::string &command,
                          std::vector<std::string> args);

template<typename... Args>
bool execute_command(std::ostream &outstream, std::ostream &errorStream, const std::string &command, Args... args)
{
    return execute_command_list(outstream, errorStream, command, {std::forward<Args>(args)...});
}
