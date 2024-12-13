
#pragma once
#include <filesystem>
#include <sstream>
#include "compiler/CompilerOptions.h"

void compile_file(CompilerOptions options, std::filesystem::path inputPath, std::ostream &errorStream,
                  std::ostream &outputStream);
