#pragma once
#include <filesystem>

#include <string>
#include <vector>

bool pascal_link_modules(const std::filesystem::path &baseDir, const std::string &program_name, std::vector<std::string> flags, std::vector<std::string> object_files);
