#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <sys/stat.h>

#include "file.h"
#include "state.h"
#include "parser.h"
#include "utils.h"
#include "debug.h"
#include "rfscss.h"
#include "specification.h"
#include "parser_exception.h"
#include "wildcard.h"

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: rfscss <file | -p> [ -l <path> | -i <rfscss> | -e <path> | -w | -t ]\n";
        std::cout << "  --from-pipe [-p]: read from stdin.\n";
        std::cout << "  --tidy [-t]: tidy the selector when using ? to extract into a file.\n";
        std::cout << "  --enable-warnings [-w]: enable warnings.\n";
        std::cout << "  --list [-l] <path>: list all of the selectors in a file.\n";
        std::cout << "  --inline-rfscss [-i] <rfscss>: use <rfscss> as the specification.\n";
        std::cout << "  --export-imports [-e] <path>: write @import rules for every extracted rule, and save it at <path>"
                  << std::endl;

        return 0;
    }


    // Get options
    std::map<std::string, std::string> options;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--tidy" || arg == "-t") {
            options["tidy"] = "true";
        } else if (arg == "--enable-warnings" || arg == "-w") {
            options["warnings"] = "true";
        } else if (arg == "--export-imports" || arg == "-e") {
            if (!argv[i + 1]) {
                std::cerr << "error: --export-imports [-e] is missing an output path"
                          << std::endl;
                return 2;
            }

            options["export-imports"] = std::string(argv[i + 1]);
        } else if (arg == "--inline-rfscss" || arg == "-i") {
            if (!argv[i + 1]) {
                std::cerr << "error: --inline-rfscss [-i] takes an rfscss spec as argument"
                          << std::endl;
                return 2;
            }

            options["inline-rfscss"] = std::string(argv[i + 1]);
        } else if (arg == "--list" || arg == "-l") {
            if (!argv[i + 1]) {
                std::cerr << "error: --list [-i] takes an output path as argument"
                          << std::endl;
                return 2;
            }

            options["list"] = std::string(argv[i + 1]);
        } else if (arg == "--from-pipe" || arg == "-p") {
            options["from-pipe"] = "true";
        }
    }

    std::string file_path = argv[1];
    std::string file_name;
    std::string workspace;

    // Declare the input string
    std::string input;
    if (options.count("from-pipe")) {
        // Read all from stdin
        workspace = '.';
        file_name = "stdin";
        file_path = "pipe";

        // Read the input from stdin using an iterator
        input = std::string(
            std::istreambuf_iterator<char>(std::cin), {}
        );
    } else {
        workspace = file_path;
        // Read from file
        std::string::size_type delim_pos = workspace.find_last_of("/");

        if (delim_pos != std::string::npos) {
            workspace = workspace.substr(0, delim_pos);
        }

        // If workspace is equal to file_path, then the file is in the same directory
        if (workspace == file_path) {
            // Set the workspace to the current directory
            workspace = '.';
        }

        // Get the file name excluding path
        delim_pos = file_name.find_last_of("/");
        if (delim_pos != std::string::npos) {
            file_name = file_name.substr(delim_pos + 1);
        }

        input = File::read(file_path);
    }

    // Check if input is empty
    if (input.empty()) {
        return 1;
    }

    // Open the .rfscss specification. Should be available at "workspace"
    Specification spec;

    bool is_spec_available = true;
    bool list_selectors = false;

    std::string spec_path = workspace + "/.rfscss";
    std::string spec_input;

    // Check if options["inline-rfscss"] is not set
    if (
           options.count("inline-rfscss")
        && options["inline-rfscss"].empty()
        || !options.count("inline-rfscss")
    ) {
        // If no inline specification is provided, try to read the .rfscss
        // file from the workspace
        spec_input = File::read(spec_path);
    } else if (options.count("inline-rfscss") && options["inline-rfscss"] != "") {
        // If an inline specification is provided, use it
        spec_input = std::string(
            options["inline-rfscss"].begin(),
            options["inline-rfscss"].end()
        );
    }

    if (options.count("list")) {
        list_selectors = true;
        std::cout << "rfscss - creating a list of selectors based in the input...\n";
    } else if (spec_input.empty()) {
        std::cout << "rfscss - no specification provided.\n"
                     "run rfscss with flag --list [-l] to list all "
                     "of the rules in the input"
                  << std::endl;

        return 1;
    }

    if (spec_input.empty()) {
        is_spec_available = false;
    } else {
        std::cout << "rfscss - parsing specification provided" << std::endl;

        // Create a new state for the specification parser
        auto spec_state = std::make_unique<State>();

        // Parse the specification
        try {
            spec = rfscss_spec::parse_spec(spec_state, spec_input);
        } catch (parser_exception& e) {
            e.print(file_path);
            return 1;
        }
    }

    auto state = std::make_unique<State>();
    Parser parser(state);

    // Parse the file
    try {
        parser.parse_input(workspace, input);
    } catch (parser_exception& e) {
        e.print(file_path);
        return 1;
    }

    if (is_spec_available && !list_selectors) {
        // Write the refactored output
        // Create a .scss file for every style rule
        int captured_selectors_amount = 0;
        bool selectors_without_match = false;
        bool is_match = false;

        std::vector<std::string> captures;
        std::vector<std::string> import_paths;

        for (
            int rule_index = 0;
            rule_index < state->selectors.size();
            rule_index++
        ) {
            // Trim the rule name
            auto rule_name =
                Utils::trim(state->selectors[rule_index]);

            for (
                int match_index = 0;
                match_index < spec.match_strings.size();
                match_index++
            ) {
                // Attempt to match this rule with the current pattern
                auto& match_string = spec.match_strings[match_index];
                is_match = Wildcard::match(
                    match_string,
                    rule_name,
                    captures
                );

                // If it's a match then we can proceed
                if (is_match) {
                    // Increase the amount of captured selectors
                    captured_selectors_amount++;

                    // Get the output path for this pattern
                    auto& file_path = spec.output_paths[match_index];

                    // Replace ? for the captured string at `i` position
                    if (!captures.empty()) {
                        for (size_t i = 0; i < file_path.size(); ++i) {
                            if (file_path[i] == '?') {
                                if (options.count("tidy")) {
                                    // Tidy the output path to make sure
                                    // it's a valid path
                                    Utils::tidy(captures[0]);

                                    file_path.replace(i, 1, captures[0]);
                                } else {
                                    file_path.replace(i, 1, captures[0]);
                                }

                                if (options.count("warnings")) {
                                    // If its not a valid path, warn the user
                                    if (!File::is_valid_path(file_path)) {
                                        std::cerr << "rfscss - warning: '"
                                                  << file_path << "' path could possibly be invalid"
                                                  << std::endl;
                                    }
                                }

                                // Advance the cursor so we continue looking for the next
                                // ? in the output path
                                i += captures[0].size() - 1;

                                // Erase the replaced string from the list of captures
                                captures.erase(captures.begin());
                            }
                        }
                    }

                    if (options["export-imports"] != "") {
                        // Add the import path to the list of import paths
                        if (
                            std::find(
                                import_paths.begin(),
                                import_paths.end(), file_path
                            ) == import_paths.end()
                        ) {
                            std::cout << ". added import path " << file_path << '\n';
                            import_paths.push_back(file_path);
                        }
                    }

                    // Get the content for this rule
                    auto& content = state->selectors[rule_index];

                    // If the content is not empty, that means this rule
                    // is a style rule
                    if (state->content[rule_index] != "") {
                        // Append the content and two newlines to separate between rules
                        content += + "{" + state->content[rule_index] + "}\n\n";
                    }

                    // Append the content to the corresponding file
                    File::place_in(workspace + "/" + file_path, content);

                    break;
                }
            }

            // If the rule is not a match, we'll warn the user
            // that there are selectors without matches
            if (!is_match) {
                selectors_without_match = true;
            }
        }

        if (selectors_without_match && options.count("warnings")) {
            std::cerr << "rfscss - warning: found rules without match.\n"
                      << "declare a '%' or '?' rule if you wish to capture all rules"
                      << std::endl;
        }

        if (options["export-imports"] != "") {
            // Export a list of @import rules to the path specified
            // by the user
            std::cout << ". exporting @imports at " << options["export-imports"] << std::endl;

            std::stringstream ss;
            for (std::string import_path : import_paths) {
                ss << "@import \"" << import_path << "\";" << std::endl;
            }

            File::place_in(options["export-imports"], ss.str());
        }

        std::cout << "rfscss - "
                  << "captured " << captured_selectors_amount
                  << " out of " << state->selectors.size() << " selectors"
                  << std::endl;
    }

    if (list_selectors) {
        // List all of the selectors in the input
        std::stringstream ss;

        for (int i = 0; i < state->selectors.size(); i++) {
            auto& rule_name = state->selectors[i];

            // Trim the selector name
            rule_name = Utils::trim(rule_name);
            Utils::replace(rule_name, "\n", " ");

            ss << rule_name << "\n";
        }

        File::place_in(workspace + "/" + options["list"], ss.str());

        std::cout << "rfscss - list saved at " << workspace << "/" << options["list"] << std::endl;
    }

    return 0;
}
