#pragma once

#include <ostream>
#include <string>
#include <vector>

bool execute_command_list(std::ostream &outstream, const std::string &command, std::vector<std::string> args);

template <typename... Args>
bool execute_command(std::ostream &outstream, const std::string &command, Args... args)
{
    return execute_command_list(outstream, command, {std::forward<Args>(args)...});
}