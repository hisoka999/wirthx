#include "compare.h"

#include <algorithm>
#include <cctype>
#include <string>

bool ichar_equals(const char a, const char b)
{
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
}
bool iequals(const std::string_view &a, const std::string_view &b) { return std::ranges::equal(a, b, ichar_equals); }


std::string to_lower(const std::string &a)
{
    std::string result = a;
    std::ranges::transform(result, result.begin(), [](const unsigned char c) { return std::tolower(c); });
    return result;
}
