#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include "parser.h"
#include "file.h"

int Parser::check_char(char c) {
    // Return 0 if the character is a dot
    // Return 1 if the character is an opening brace
    // Return 2 if the character is a closing brace
    // Return 3 if the character is a whitespace
    // Return 4 if the character is a forward slash
    // Return 5 if the character is an asterisk
    // Return 6 if the character is a newline

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
    } else {
        return -1;
    }
}

void Parser::parse_input(
    std::shared_ptr<State> state,
    std::string file_path,
    std::string workspace,
    std::vector<char> input 
) {
    for (auto c : input) {
        // If current position is above 0, then set the previous character
        // to the current character
        if (state->curr_pos > 0) {
            state->last_char = state->curr_char;
        }

        // Check for the character
        int check = this->check_char(c);

        // Increment the position
        state->curr_pos++;
        // Set the current character
        state->curr_char = c;

        // If the character is a newline, increment current line number
        if (check == 6) {
            state->curr_line++;
        }

        // Check if is_comment is true
        if (state->is_comment) {
            std::cout << "Current character is " << c << std::endl;
            std::cout << "Is multiline comment set to " << state->is_multiline_comment << std::endl;
            // If the character is an forward slash, we are in hold comment mode
            // and the last character was an asterisk, set is_comment multiline
            // to false
            if (check == 4 && state->hold_comment && state->last_char == '*') {
                state->is_comment = false;
                state->is_multiline_comment = false;
            } else if (state->hold_comment) {
                // Comment marker didn't complete
                std::cout << "Didnt drop comment because " << state->last_char << c << std::endl;
                state->hold_comment = false;
            }

            // If the current character is a newline and we are not in a multiline comment,
            // set is_comment to false
            if (check == 6 && !state->is_multiline_comment) {
                state->is_comment = false;
            }

            // If the current character is an asterisk and we are in a multiline comment,
            // set hold comment to true
            if (check == 5 && state->is_multiline_comment) {
                std::cout << "Multiline holding because of " << c << std::endl;
                state->hold_comment = true;
            }

            // If the character is an asterisk and we are in a comment,
            // set hold comment to true
            if (check == 5 && state->is_comment && !state->hold_comment) {
                std::cout << "Possibly dropping comment" << std::endl;
                state->hold_comment = true;
            }
        }

        
        if (!state->is_comment) {
            // If the character is an asterisk, we are in hold comment mode
            // and the last character was a forward slash, set is_comment multiline
            // to true
            if (check == 5 && state->hold_comment && state->last_char == '/') {
                state->is_comment = true;
                state->is_multiline_comment = true;
            } else  if (check == 4 && state->hold_comment && state->last_char == '/') {
                // If the character is a forward slash, we are in hold comment mode
                // and the last character was a forward slash, set is_comment to true
                state->is_comment = true;
            } else {
                // Comment marker didn't complete
                state->hold_comment = false;
            }
        }
        
        // If the character is a forward slash, set hold comment to true
        if (check == 4 && !state->hold_comment) {
            state->hold_comment = true;
        }

        // Check if capturing is false, except for comments
        if (!state->capturing && !state->hold_comment) {
            // If the character is a dot, add the identifier to the identifiers vector
            if (check == 0) {
                state->identifiers.push_back(std::string());
                // Assign this identifier an id and push it to the identifiers_id vector
                state->identifier_ids.push_back(state->identifiers.size() - 1);
                // Set getting identifier to true so that the next character is added to the identifier
                state->getting_identifier = true;
                // Add the position of the identifier to the identifier_pos vector
                state->identifier_pos.push_back(state->curr_pos);
                // Add the line number of the identifier to the identifier_line vector
                state->identifier_line.push_back(state->curr_line);
                continue;
            }

            // If the character is an opening brace, capture all the characters until the closing brace
            if (check == 1) {
                state->capturing = true;
                // Set getting identifier to false to denote that we are now getting the content
                state->getting_identifier = false;
                // Add a new string to state->content
                state->content.push_back(std::string());
                // Increment level by one
                state->levels++;
                continue;
            }

            // Check if getting_identifier is true
            // If its true, add the character to the last string of the identifiers vector
            if (state->getting_identifier) {
                std::cout << "Pushing identifier" << state->identifiers.back() << std::endl;
                state->identifiers.back() += c;
            }
        } else if (state->capturing) {
            // If the character is an opening brace, increment level by one
            if (check == 1 && !state->is_comment) {
                state->levels++;
            }

            // If the character is a closing brace, decrement level by one
            if (check == 2 && !state->is_comment) {
                state->levels--;
            }

            // If the level is 0, capture is false
            if (state->levels == 0 && !state->is_comment) {
                state->capturing = false;
                continue;
            }

            // Add the character to the last string of the content vector
            state->content[state->content.size() - 1] += c;
        }
    }
    
    // If state->levels is not 0, the program did not parse correctly
    // (i.e. there is a closing brace without an opening brace or vice versa)
    if (state->levels != 0) {
        std::cout << "rfScss - Unable to refactor file: " << file_path << std::endl; 
        std::cout << "rfScss - Unbalanced braces in";
        // (Of course, the last identifier is not necessarily the last one in the file,
        // but rather the last one that was successfully parsed)
        std::cout << " ."  << state->identifiers[state->identifiers.size() - 1] 
                << " | " << file_path << " [ line: ";
        // Print last identifier line number
        std::cout << state->identifier_line[state->identifier_line.size() - 1];
        // Print last identifier position
        std::cout << ", character:"   << state->identifier_pos[state->identifier_pos.size() - 1] 
                << " ]"   << std::endl;

        // Set the state to error
        state->error = "Unbalanced braces";

        return;
    }

    // Return a pointer to the state
    return;
}