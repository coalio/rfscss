#include <string>
#include <vector>
#include <memory>
#include "error_definitions.h"
#include "parser_exception.h"
#include "rfscss.h"
#include "specification.h"
#include "state.h"
#include "utils.h"

Specification rfscss_spec::parse_spec(
    std::unique_ptr<State>& state,
    std::string input
) {
    struct Specification spec;

    for (auto c : input) {
        if (state->curr_pos > 0) {
            state->last_char = state->curr_char;
        }

        state->curr_col++;
        state->curr_pos++;
        state->curr_char = c;

        if (!state->capturing_block) {
            if (state->expecting_rule_or_selector) {
                if (!Utils::is_whitespace(state->curr_char)) {
                    // We're gonna capture the first half of the rule
                    // (selector)->path
                    state->getting_selector = true;
                    state->expecting_rule_or_selector = false;
                    spec.match_strings.emplace_back("");
                } else {
                    // There's nothing to do for now, until we find a non-whitespace char
                    continue;
                }
            }

            if (state->getting_selector) {
                // Push to the current selector string
                spec.match_strings.back() += state->curr_char;
            }

            if (state->last_char == '-' && state->curr_char == '>') {
                if (spec.match_strings.back().size() == 2) {
                    // '->' isn't valid if there's no selector to match against
                    // size() == 2 means that it only has captured the '-' and '>'
                    char error_msg[100];
                    sprintf(
                        error_msg,
                        ERR_SPECIFICATION_MISSING_SELECTOR,
                        state->curr_line
                    );

                    throw parser_exception(
                        state->curr_line,
                        state->curr_col,
                        state->curr_pos,
                        error_msg,
                        "Invalid specification"
                    );
                }

                state->getting_selector = false;
                state->capturing_block = true;
                spec.match_strings.back() =
                    spec.match_strings.back()
                        .substr(0, spec.match_strings.back().size() - 2);

                spec.output_paths.emplace_back("");
            }

            if (state->curr_char == '\n') {
                char error_msg[100];
                sprintf(
                    error_msg,
                    ERR_SPECIFICATION_MISSING_PATH,
                    state->curr_line,
                    spec.match_strings.back().c_str()
                );

                throw parser_exception(
                    state->curr_line,
                    state->curr_col,
                    state->curr_pos,
                    error_msg,
                    "Invalid specification"
                );
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
                char error_msg[100];
                sprintf(
                    error_msg,
                    ERR_SPECIFICATION_MISSING_PATH,
                    state->curr_line,
                    spec.match_strings.back().c_str()
                );

                throw parser_exception(
                    state->curr_line,
                    state->curr_col,
                    state->curr_pos,
                    error_msg,
                    "Invalid specification"
                );
            }

            spec.output_paths.back() += state->curr_char;
        }
    }

    return spec;
}