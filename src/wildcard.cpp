#include <string>
#include "wildcard.h"
#include "state.h"
#include "debug.h"

int8_t Wildcard::check_char(char c) {
    // Return 0 if the character is not special
    // Return 1 if the character is a %
    // Return 2 if the character is a ?
    // Return 3 if the character is a _
    // Return 4 if the character is a "\"

    if (c == '%') {
        return 1;
    } else if (c == '?') {
        return 2;
    } else if (c == '_') {
        return 3;
    } else if (c == '\\') {
        return 4;
    }

    return 0;
}

bool Wildcard::match(
    std::string pattern,
    std::string compare,
    std::vector<std::string> &captures
) {
    struct Wildcard::WildcardState state;
    bool is_match = true;
    bool ignore_captures;

    if (&captures == &DEFAULT_VECTOR) {
        ignore_captures = true;
    }

    while (state.pivot_curr_pos <= compare.size() - 1) {
        if (state.curr_pos > 0) {
            state.last_sign = state.curr_sign;
        }

        if (state.curr_pos > pattern.size() - 1) {
            if (state.pivot_curr_pos < compare.size()) {
                is_match = false;
            }

            break;
        }

        state.curr_sign = check_char(pattern[state.curr_pos]);
        LOG("debug: " << pattern << "\n\7compare: " << compare << " pivot: " << state.pivot_curr_pos << " curr: " << state.curr_pos << " last: " << state.last_sign << " curr_sign: " << state.curr_sign);
        LOG("match: " << is_match);
        LOG("current pattern char:" << pattern[state.curr_pos]);
        LOG("current compare char:" << compare[state.pivot_curr_pos]);
        LOG(std::endl);

        if (state.curr_sign == 4 && !state.take_next_literally) {
            LOG("Entered sign 4");
            state.take_next_literally = true;
            // Skip this one and continue on the next
            state.curr_pos++;

            // If this is next to % or ?, the character the backslash is escaping
            // should be the match point, not the backslash itself
            if (state.last_sign == 1 || state.last_sign == 2) {
                state.match_position_point = state.curr_pos;
            }
        }

        if (state.curr_sign == 0 || state.take_next_literally) {
            state.take_next_literally = false;
            LOG("Entered sign 0");
            // The "pivot" current position does not match the compare
            // current position
            if (pattern[state.curr_pos] != compare[state.pivot_curr_pos]) {
                is_match = false;

                if (state.on_pivot_wildcard) {
                    state.curr_pos = state.match_position_point;
                    state.on_pivot_position = true;
                } else {
                    // If it fails an absolute exact match without a wildcard, the string
                    // will never match the pattern
                    LOG("Looks like we failed an absolute match");
                    break;
                }
            } else if (state.on_pivot_wildcard) {
                state.on_pivot_position = false;
                is_match = true;
            }
        }

        if (state.curr_sign == 1 && !state.take_next_literally) {
            LOG("Entered sign 1");
            if (!is_match && state.last_sign == 0) {
                // If the current character is a %, and the string is not a match,
                // the  string will never match the pattern
                break;
            }

            state.curr_pos++;
            if (!pattern[state.curr_pos]) {
                // If there is nothing after %, exit the loop
                break;
            }

            // is_match is false until the following characters are met
            // remains false if they were never met
            is_match = false;
            state.on_pivot_position = true;
            state.on_pivot_wildcard = true;
            state.match_position_point = state.curr_pos;
        }

        if (state.curr_sign == 2 && !state.take_next_literally) {
            LOG("Entered sign 2");
            if (!is_match && state.last_sign == 0) {
                break;
            }

            state.curr_pos++;
            if (!pattern[state.curr_pos] && is_match == true && !ignore_captures) {
                // If there is nothing after ?,
                // anything that comes will be valid anyways, so set
                // is_match to true and capture everything next then break
                state.captures.emplace_back("");
                state.captures.back() += compare.substr(state.pivot_curr_pos);
                break;
            }

            // is_match is false until the following characters are met
            // remains false if they were never met
            is_match = false;
            state.on_pivot_position = true;
            state.on_pivot_wildcard = true;
            state.match_position_point = state.curr_pos;

            if (!ignore_captures) {
                state.is_capturing = true;
                state.captures.emplace_back("");
            }
        }

        if (state.curr_sign == 3 && !state.take_next_literally) {
            LOG("Entered sign 3");
            if (!compare[state.pivot_curr_pos]) {
                is_match = false;
            }
        }


        if (state.on_pivot_position && state.is_capturing) {
            state.captures.back() += compare[state.pivot_curr_pos];
        }

        LOG("is_match: " << is_match);
        LOG("on_pivot_position: " << state.on_pivot_position);
        LOG("on_pivot_wildcard: " << state.on_pivot_wildcard);
        LOG("match_position_point: " << state.match_position_point);
        LOG("curr_pos: " << state.curr_pos);
        LOG("pivot_curr_pos: " << state.pivot_curr_pos);
        LOG("last_sign: " << state.last_sign);
        LOG("curr_sign: " << state.curr_sign);
        LOG("is_capturing: " << state.is_capturing);
        LOG("take_next_literally: " << state.take_next_literally);
        LOG("finish iteration no. " << state.pivot_curr_pos << std::endl);
        state.pivot_curr_pos++;
        // Only update current position if we're not on a pivot position.
        if (!state.on_pivot_position) {
            state.curr_pos++;
        }
    }

    captures = state.captures;

    // If we left the loop and the pattern is still not finished, set is_match to false
    if (state.curr_pos < pattern.size() - 1) {
        is_match = false;
    }

    return is_match;
}

#if DEBUG
void _test_wildcards() {
    bool test_a = Wildcard::match("abcb", "abcb");
    bool test_b = Wildcard::match("a_cb", "abcb");
    bool test_c = Wildcard::match("abcb_", "abcb");
    bool test_d = Wildcard::match("a%c%ch", "a nice match");
    bool test_e = Wildcard::match("a%c%ch", "a nice matcx");
    bool test_f = Wildcard::match("5\\% great", "5% great");
    bool test_g = Wildcard::match("an%", "ants");

    // create a vector of strings that saves the capture
    std::vector<std::string> test_h_captures;
    bool test_h = Wildcard::match("hello ?ld", "hello world", test_h_captures);

    std::vector<std::string> test_i_captures;
    bool test_i = Wildcard::match("hello ?", "hello world", test_i_captures);

    std::vector<std::string> test_j_captures;
    bool test_j = Wildcard::match("hi ?, im ?", "hi hungry, im dad", test_j_captures);

    std::vector<std::string> test_k_captures;
    bool test_k = Wildcard::match("an ? for an ? only leaves the whole ? blind", "an eye for an eye only leaves the whole world blind", test_k_captures);

    bool test_l = Wildcard::match("%", "anything");

    std::vector<std::string> test_m_captures;
    bool test_m = Wildcard::match("?", "everything", test_m_captures);

    std::vector<std::string> test_n_captures;
    bool test_n = Wildcard::match(".?\\ ", ".class-class class", test_n_captures);

    std::vector<std::string> test_o_captures;
    bool test_o = Wildcard::match(".? ", ".class-class class", test_o_captures);

    LOG("abcb == abcb: " << test_a);
    LOG("a_cb == acbc: " << test_b);
    LOG("abcb_ == acbc: " << test_c);
    LOG("a%c%ch == a nice match: " << test_d);
    LOG("a%c%ch == a nice matcx: " << test_e);
    LOG("5\\% great == 5% great: " << test_f);
    LOG("an% == ants: " << test_g);
    LOG("hello ?ld == hello world: " << test_h);
    for (std::string capture : test_h_captures) {
        LOG("test_h capture: " << capture);
    }
    LOG("hello ? == hello world: " << test_i);
    for (std::string capture : test_i_captures) {
        LOG("test_i capture: " << capture);
    }
    LOG("hi ?, im ? == hi hungry, im dad: " << test_j);
    for (std::string capture : test_j_captures) {
        LOG("test_j capture: " << capture);
    }
    LOG("an ? for an ? only leaves the whole ? blind == an eye for an eye only leaves the whole world blind: " << test_k);
    for (std::string capture : test_k_captures) {
        LOG("test_k capture: " << capture);
    }

    LOG("% = anything: " << test_l);
    LOG("? = everything: " << test_m);
    for (std::string capture : test_m_captures) {
        LOG("test_m capture: " << capture);
    }
    LOG(".?\\  = .class-class class: " << test_n);
    for (std::string capture : test_n_captures) {
        LOG("test_n capture: " << capture);
    }
    LOG(".?  = .class-class class: " << test_o);
    for (std::string capture : test_o_captures) {
        LOG("test_o capture: " << capture);
    }
}
#endif
