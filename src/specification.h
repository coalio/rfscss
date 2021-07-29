#pragma once
#include <string>
#include <vector>

struct Specification {
    std::vector<std::string> match_strings;
    std::vector<std::string> output_paths;
};