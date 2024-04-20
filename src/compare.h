#pragma once
#include <string_view>
#include <cctype>
#include <algorithm>

bool ichar_equals(char a, char b);
bool iequals(const std::string_view &a, const std::string_view &b);