#include "compare.h"

#include <algorithm>
#include <cctype>
#include <string>

bool ichar_equals(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
}
bool iequals(const std::string_view &a, const std::string_view &b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), ichar_equals);
}


std::string to_lower(const std::string &a)
{
    std::string result = a;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) { return std::tolower(c); });
    return result;
}
