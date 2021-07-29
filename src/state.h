#pragma once
#include <string>
#include <vector>
#include "error.h"
#include "debug.h"

// Current state of the automaton
struct State {
    int curr_pos = 0;
    int levels = 0;
    int curr_line = 1;
    int curr_col = 0; 
    int8_t curr_sign = 0;
    char curr_char;
    char last_char;
    bool is_comment = false;
    bool is_multiline_comment = false;
    bool hold_comment = false;
    bool capturing_block = false;
    bool expecting_rule_or_selector = true;
    bool getting_selector = false;

    // Last error
    Error * error;
    // Vector of strings
    std::vector<std::string> selectors;
    // Vector of ints containing the selectors ids
    std::vector<int> selector_ids;
    // Vector of ints containing the cursor position of the selectors
    std::vector<int> selector_pos;
    // Vector of ints containing the line number of the selectors
    std::vector<int> selector_line;
    // Vector of strings containing the class content as a string
    std::vector<std::string> content;
    
    State() : error(new Error()) {
        LOG("Created a new state instance");
    };
};