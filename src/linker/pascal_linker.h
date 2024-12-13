#pragma once
#include <filesystem>

#include <ostream>
#include <string>
#include <vector>

bool pascal_link_modules(std::ostream &errStream, const std::filesystem::path &baseDir, const std::string &program_name,
                         std::vector<std::string> flags, std::vector<std::string> object_files);
