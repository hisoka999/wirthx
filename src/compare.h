#pragma once
#include <algorithm>
#include <cctype>
#include <string_view>
#include <string>

bool ichar_equals(char a, char b);
bool iequals(const std::string_view &a, const std::string_view &b);
std::string to_lower(const std::string &a);
