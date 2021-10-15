#include <string>
#include "utils.h"

std::string Utils::ltrim(const std::string str) {
    size_t start = str.find_first_not_of(TRIM_ABLE);
    return (start == std::string::npos) ? "" : str.substr(start);
}

std::string Utils::rtrim(const std::string str) {
    size_t end = str.find_last_not_of(TRIM_ABLE);
    return (end == std::string::npos) ? "" : str.substr(0, end + 1);
}

std::string Utils::trim(const std::string str) {
    return rtrim(ltrim(str));
}

std::string Utils::tidy(std::string str) {
    str.erase(std::remove_if(str.begin(), str.end(), [](char c) {
        return (!std::isprint(c) || !(isalnum(c) || c == ',' || c == ' '));
    }), str.end());

    str = replace(str, ",", "_");
    str = replace(str, " ", "_");

    return str;
}

std::string Utils::replace(std::string str, std::string find, std::string replace) {
    size_t pos = 0;

    while ((pos = str.find(find, pos)) != std::string::npos) {
        str.replace(pos, find.length(), replace);
        pos += replace.length();
    }

    return str;
}

bool Utils::is_whitespace(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}