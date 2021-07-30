#pragma once
#include <string>
#include <vector>
#include "debug.h"

namespace Wildcard {
    int8_t check_char(char c);
    // Compares the <pattern> to a <compare> string
    // o(NM) complexity: N compare size M pattern size
    bool match(std::string pattern, std::string compare);
    bool match(
        std::string pattern,
        std::string compare,
        std::vector<std::string> &captures
    );

    struct WildcardState {
        int virtual_curr_pos = 0;
        int last_sign = 0;
        int curr_sign;
        int curr_pos = 0;
        int match_position_point;
        bool on_virtual_wildcard = false;
        bool on_virtual_position = false;
        bool is_capturing = false;
        bool take_next_literally = false;

        std::vector<std::string> captures;
    };
};

#if DEBUG
void _test_wildcards();
#endif