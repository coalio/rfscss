#include <string>
#include <vector>
#include <memory>
#include "error_definitions.h"
#include "error.h"
#include "rfscss.h"
#include "specification.h"
#include "state.h"
#include "utils.h" 

Specification rfscss_spec::parse_spec(std::shared_ptr<State> state, std::vector<char> input) {
    struct Specification spec;

    for (auto c : input) {
        if (state->curr_pos > 0) {
            state->last_char = state->curr_char;
        }

        state->curr_col++;
        state->curr_pos++;
        state->curr_char = c;

        if (!state->capturing_block) {
            if (state->expecting_rule_or_selector && !Utils::is_whitespace(state->curr_char)) {
                state->getting_selector = true;
                state->expecting_rule_or_selector = false;
                spec.match_strings.emplace_back("");
            }

            if (state->getting_selector) {
                spec.match_strings.back() += state->curr_char;
            }

            if (state->last_char == '-' && state->curr_char == '>') {
                state->getting_selector = false;
                state->capturing_block = true;
                spec.match_strings.back() = 
                    spec.match_strings.back()
                        .substr(0, spec.match_strings.back().size() - 2);

                spec.output_paths.emplace_back("");
            }

            if (state->curr_char == '\n') {
                char error_msg[100]; sprintf(
                    error_msg, 
                    ERR_SPECIFICATION_MISSING_PATH, 
                    state->curr_line, 
                    spec.match_strings.back().c_str()
                );

                Error * error = new Error();

                error->kind = "Invalid specification";
                error->message = error_msg;
                error->line = state->curr_line;
                error->column = state->curr_col;
                error->at_char = state->curr_pos;

                state->error = error;
                return spec;
            }
        } else {
            if (state->curr_char == '\n') {
                state->curr_line++;
                state->curr_col = 0;
                state->capturing_block = false;
                state->expecting_rule_or_selector = true;
                continue;
            }

            if (state->last_char == '-' && state->curr_char == '>') {
                char error_msg[100]; sprintf(
                    error_msg, 
                    ERR_SPECIFICATION_MISSING_PATH, 
                    state->curr_line, 
                    spec.match_strings.back().c_str()
                );

                Error * error = new Error();

                error->kind = "Invalid specification";
                error->message = error_msg;
                error->line = state->curr_line;
                error->column = state->curr_col;
                error->at_char = state->curr_pos;

                state->error = error;
                return spec;
            }
            
            spec.output_paths.back() += state->curr_char;
        }
    }

    return spec;
}