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
    // Remove all special characters from str
    str.erase(std::remove_if(str.begin(), str.end(), [](char c) {
        return (!std::isprint(c) || !(isalnum(c) || c == '.'));
    }), str.end());
    // Replace space with underscore
    str.replace(str.begin(), str.end(), ' ', '_');

    return str;
}

bool Utils::is_whitespace(char c) {
    // Check if character is whitespace
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}