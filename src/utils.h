#pragma once
#include <iostream>
#include <string>
#include <algorithm>


namespace Utils {
    const std::string TRIM_ABLE = " \n\r\t\f\v";
    
    std::string ltrim(const std::string &str) {
        size_t start = str.find_first_not_of(TRIM_ABLE);
        return (start == std::string::npos) ? "" : str.substr(start);
    }
    
    std::string rtrim(const std::string &str) {
        size_t end = str.find_last_not_of(TRIM_ABLE);
        return (end == std::string::npos) ? "" : str.substr(0, end + 1);
    }
    
    std::string trim(const std::string &str) {
        return rtrim(ltrim(str));
    }
}