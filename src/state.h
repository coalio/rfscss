#pragma once
#include <string>
#include <vector>

// Current state of the automaton
struct State {
    int curr_pos = 0;
    int levels = 0;
    int curr_line = 0; 
    char curr_char;
    char last_char;
    bool is_comment = false;
    bool is_multiline_comment = false;
    bool hold_comment = false;
    bool capturing = false;
    bool getting_identifier = false;

    // Last error
    std::string error;
    // Vector of strings
    std::vector<std::string> identifiers;
    // Vector of ints containing the identifiers ids
    std::vector<int> identifier_ids;
    // Vector of ints containing the position of the identifiers
    std::vector<int> identifier_pos;
    // Vector of ints containing the line number of the identifiers
    std::vector<int> identifier_line;
    // Vector of strings containing the class content as a string
    std::vector<std::string> content;
};