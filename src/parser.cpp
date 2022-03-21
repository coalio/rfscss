#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include "error_definitions.h"
#include "error.h"
#include "file.h"
#include "utils.h"
#include "parser.h"

int8_t Parser::check_char(char c) {
    // Return 0 if the character is a dot
    // Return 1 if the character is an opening brace
    // Return 2 if the character is a closing brace
    // Return 3 if the character is a whitespace
    // Return 4 if the character is a forward slash
    // Return 5 if the character is an asterisk
    // Return 6 if the character is a newline
    // Return 7 if the character is a semicolon
    // Return 8 if the character is a number sign

    if (c == '.') {
        return 0;
    } else if (c == '{') {
        return 1;
    } else if (c == '}') {
        return 2;
    } else if (c == ' ') {
        return 3;
    } else if (c == '/') {
        return 4;
    } else if (c == '*') {
        return 5;
    } else if (c == '\n') {
        return 6;
    } else if (c == ';') {
        return 7;
    }  else if (c == '#') {
        return 8;
    }

    return -1;
}

void Parser::next(char c) {
    // If current cursor is above 1, then set the previous character
    // to the current character
    if (state->curr_pos > 0) {
        state->last_char = state->curr_char;
    }

    state->curr_pos++;

    // Increment the cursor
    // Set the current character
    state->curr_char = c;
    state->curr_sign = this->check_char(c);
}

void Parser::increment_cursor() {
    // If current character is a newline, increment the line number
    if (state->curr_sign == 6) {
        state->curr_line++;
        state->curr_col = 1;
    } else {
        state->curr_col++;
    }
}

void Parser::in_comment() {
    // Check if is_comment is true
    if (state->is_comment) {
        // If the character is an forward slash, we are in hold comment mode
        // && the last character was an asterisk, set is_comment multiline
        // to false
        if (state->curr_sign == 4 && state->hold_comment && state->last_char == '*') {
            state->is_comment = false;
            state->is_multiline_comment = false;
        } else if (state->hold_comment) {
            // Comment marker didn't complete
            state->hold_comment = false;
        }

        // If the current character is a newline && we are not in a multiline comment,
        // set is_comment to false
        if (state->curr_sign == 6 && !state->is_multiline_comment) {
            state->is_comment = false;
        }

        // If the current character is an asterisk && we are in a multiline comment,
        // set hold comment to true
        if (state->curr_sign == 5 && state->is_multiline_comment) {
            state->hold_comment = true;
        }

        // If the character is an asterisk && we are in a comment,
        // set hold comment to true
        if (state->curr_sign == 5 && state->is_comment && !state->hold_comment) {
            state->hold_comment = true;
        }
    }
}

void Parser::find_comment_marker() {
    if (!state->is_comment) {
        // If the character is an asterisk, we are in hold comment mode
        // && the last character was a forward slash, set is_comment multiline
        // to true
        if (state->curr_sign == 5 && state->hold_comment && state->last_char == '/') {
            state->is_comment = true;
            state->is_multiline_comment = true;
        } else  if (state->curr_sign == 4 && state->hold_comment && state->last_char == '/') {
            // If the character is a forward slash, we are in hold comment mode
            // && the last character was a forward slash, set is_comment to true
            state->is_comment = true;
        } else {
            // Comment marker didn't complete
            state->hold_comment = false;
        }
    }

    // If the character is a forward slash, set hold comment to true
    if (state->curr_sign == 4 && !state->hold_comment) {
        state->hold_comment = true;
    }
}

bool Parser::is_rule_or_selector() {
    // If we're expecting a selector or rule &&
    // current character is not a whitespace
    if (
        state->expecting_rule_or_selector &&
        // state->curr_sign == 0
        !Utils::is_whitespace(state->curr_char)
    ) {
        state->selectors.emplace_back("");
        state->selectors.back() += state->curr_char;
        // Assign this selector an id && push it to the selectors_id vector
        state->selector_ids.push_back(state->selectors.size() - 1);
        // Add the cursor of the selector to the selector_pos vector
        state->selector_pos.push_back(state->curr_pos);
        // Add the line number of the selector to the selector_line vector
        state->selector_line.push_back(state->curr_line);
        // Set getting selector to true so that the next character is added to the selector
        state->getting_selector = true;
        state->expecting_rule_or_selector = false;

        return true;
    }

    return false;
}

bool Parser::is_opening_brace() {
    // If the character is an opening brace, capture all the characters until the closing brace
    // Discard it if its preceded by # (interpolation)
    if (state->curr_sign == 1 && state->last_char != '#') {
        state->capturing_block = true;
        // Set getting selector to false to denote that we are now getting the content
        state->getting_selector = false;
        // Add a new string to state->content
        state->content.emplace_back("");
        // Increment level by one
        state->levels++;

        return true;
    }

    return false;
}

void Parser::push_to_selector() {
    state->selectors.back() += state->curr_char;

    if (state->curr_sign == 7) {
        state->getting_selector = false;
        state->expecting_rule_or_selector = true;
        state->content.emplace_back("");
    }
}

int Parser::check_capture_level() {
    // If the character is an opening brace, increment level by one
    if (state->curr_sign == 1 && !state->is_comment) {
        state->levels++;
    }

    // If the character is a closing brace, decrement level by one
    if (state->curr_sign == 2 && !state->is_comment) {
        state->levels--;
    }

    return state->levels;
}

bool Parser::check_parsing_errors() {
    // If state->levels is not 0, the program did not parse correctly
    // (i.e. there is a closing brace without an opening brace or vice versa)
    if (state->levels != 0) {
        char error_msg[100]; sprintf(
            error_msg, ERR_UNBALANCED_BRACES, state->selectors.back().c_str()
        );

        Error* error = new Error();

        error->at_char = state->selector_pos.back();
        error->column = 1;
        error->message = error_msg;
        error->kind = "Unbalanced braces";
        error->line = state->selector_line.back();

        // Set the state to error
        state->error = error;

        return true;
    }

    // If we are in a multiline comment when we're finished reading the file,
    // this should also be logged as an error
    if (state->is_multiline_comment) {
        char error_msg[100]; sprintf(
            error_msg, ERR_UNFINISHED_BLOCK_COMMENT, state->selectors.back().c_str()
        );

        Error* error = new Error();

        error->at_char = state->selector_pos.back();
        error->column = 1;
        error->message = error_msg;
        error->kind = "Unterminated comment";
        error->line = state->selector_line.back();

        // Set the state to error
        state->error = error;

        return true;
    }

    return false;
}

void Parser::parse_input(
    std::string workspace,
    std::vector<char> input
) {
    for (auto c : input) {
        this->next(c);
        this->increment_cursor();
        this->in_comment();
        this->find_comment_marker();

        // Check if capturing is false, except for comments
        if (!state->capturing_block && !state->hold_comment && !state->is_comment) {
            bool is_rule_or_selector = this->is_rule_or_selector();
            if (is_rule_or_selector) {
                continue;
            }

            bool is_opening_brace = this->is_opening_brace();
            if (is_opening_brace) {
                continue;
            }

            // Check if getting_selector is true
            // If its true, add the character to the last string of the selectors vector
            if (state->getting_selector) {
                this->push_to_selector();
            }
        } else if (state->capturing_block) {
            int level = this->check_capture_level();
            if (level == 0 && !state->is_comment) {
                state->capturing_block = false;
                state->expecting_rule_or_selector = true;
                continue;
            }

            // Add the character to the last string of the content vector
            state->content.back() += c;
        }
    }

    this->check_parsing_errors();

    return;
}
