#pragma once
#include "state.h"

class Parser {
public:
    int check_char(char c);
    struct State * parse_input(
        std::string file_path, 
        std::string workspace, 
        std::vector<char> input
    );
};