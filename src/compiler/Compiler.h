
#pragma once
#include <filesystem>
#include <sstream>

void compile_file(std::filesystem::path inputPath, std::ostream &errorStream, std::ostream &outputStream);
