#pragma once
#include <iostream>
#include <string>
#include <algorithm>

namespace Utils {
    const std::string TRIM_ABLE = " \n\r\t\f\v";
    // Trim a string from the left wing
    std::string ltrim(const std::string& str);
    // Trim a string from the right wing
    std::string rtrim(const std::string& str);
    // Trim a string from both wings
    std::string trim(const std::string& str);
    // Remove all the special characters
    void tidy(std::string& str);
    // Replace all occurrences of a string
    void replace(
        std::string& str,
        const std::string& find,
        const std::string& replace
    );
    // Check if the character is a whitespace
    bool is_whitespace(const char& c);
}