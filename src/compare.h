#pragma once
#include <algorithm>
#include <cctype>
#include <string>
#include <string_view>

bool ichar_equals(char a, char b);
bool iequals(const std::string_view &a, const std::string_view &b);
bool iequals(const std::string &a, const std::string &b);

std::string to_lower(const std::string &a);
