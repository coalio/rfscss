#pragma once
#include "state.h"

class Parser {
public:
    int check_char(char c);
    void parse_input(
        std::shared_ptr<State> state,
        std::string file_path, 
        std::string workspace, 
        std::vector<char> input
    );
};