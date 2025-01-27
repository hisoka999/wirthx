#pragma once


#include <algorithm>
#include <cctype>
#include <string>

#ifndef _WINDOWS
constexpr bool ichar_equals(const char a, const char b)
{
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
}
constexpr bool iequals(const std::string_view &a, const std::string_view &b)
{
    return std::ranges::equal(a, b, ichar_equals);
}

constexpr bool iequals(const std::string &a, const std::string &b) { return std::ranges::equal(a, b, ichar_equals); }

constexpr std::string to_lower(const std::string &a)
{
    std::string result = a;
    std::ranges::transform(result, result.begin(), [](const unsigned char c) { return std::tolower(c); });
    return result;
}
#else
bool ichar_equals(const char a, const char b);
bool iequals(const std::string_view &a, const std::string_view &b);

bool iequals(const std::string &a, const std::string &b);

std::string to_lower(const std::string &a);
#endif
