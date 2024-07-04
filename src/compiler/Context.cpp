#include "Context.h"

void LogError(const char *Str)
{
    fprintf(stderr, "Error: %s\n", Str);
}

llvm::Value *LogErrorV(const char *Str)
{
    LogError(Str);
    return nullptr;
}