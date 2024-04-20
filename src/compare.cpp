#include "compare.h"

#include <cctype>
#include <algorithm>
bool ichar_equals(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}
bool iequals(const std::string_view &a, const std::string_view &b)
{
    return std::equal(a.begin(), a.end(), b.begin(), b.end(), ichar_equals);
}