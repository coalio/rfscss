#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <sys/stat.h>
#include "src/file.h"
#include "src/state.h"
#include "src/parser.h"
#include "src/utils.h"
#include "src/debug.h"
#include "src/rfscss.h"
#include "src/specification.h"
#include "src/wildcard.h"

int main(int argc, char* argv[])
{
    umask(0);

    if (argc < 2) {
        std::cerr << "Usage: rfscss <file> [options: -l <path>, -i <rfscss>, -e <path>]\n";
        std::cerr << "  --tidy [-t]: tidy the selector when using ? to extract into a file.\n";
        std::cerr << "  --enable-warnings [-w]: enable warnings.\n";
        std::cerr << "  --list [-l] <path>: list all of the selectors in a file.\n";
        std::cerr << "  --inline-rfscss [-i] <rfscss>: use <rfscss> as the specification.\n";
        std::cerr << "  --export-imports [-e] <path>: write @import rules for every extracted rule, and save it at <path>\n";
        return 1;
    }

    std::string file_path = argv[1];
    std::string file_name = file_path;
    std::string workspace = file_path;

    std::string::size_type delim_pos = workspace.find_last_of("/");
    if (delim_pos != std::string::npos) {
        workspace = workspace.substr(0, delim_pos);
    }

    // If workspace is equal to file_path, then the file is in the same directory
    if (workspace == file_path) {
        // Set the workspace to the current directory
        workspace = ".";
    }

    // Get the file name excluding path
    delim_pos = file_name.find_last_of("/");
    if (delim_pos != std::string::npos) {
        file_name = file_name.substr(delim_pos + 1);
    }

    // Open the file
    std::vector<char> input = File::read(file_path);
    // Check if input is empty
    if (input.empty()) {
        return 1;
    }

    // Get options
    std::map<std::string, std::string> options;
    for (int i = 2; i < argc; i++) {
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
        }
    }

    // Open the .rfscss specification. Should be available at "workspace"
    struct Specification spec;
    bool is_spec_available = true;
    bool build_spec = false;
    std::string spec_path = workspace + "/.rfscss";
    std::vector<char> spec_input;

    // Check if options["inline-rfscss"] is not set
    if (options.count("inline-rfscss") && options["inline-rfscss"].empty() || !options.count("inline-rfscss")) {
        // If no inline specification is provided, try to read the .rfscss
        // file from the workspace
        spec_input = File::read(spec_path);
    } else if (options.count("inline-rfscss") && options["inline-rfscss"] != "") {
        // If an inline specification is provided, use it
        spec_input = std::vector<char>(options["inline-rfscss"].begin(),
                                       options["inline-rfscss"].end());
    }

    if (options.count("list")) {
        build_spec = true;
        std::cout << "rfscss - creating a list of selectors based in the input...\n";
    } else if (spec_input.empty()) {
        std::cout << "rfscss - no specification provided.\n"
                        "run rfscss with flag --list [-l] to list all "
                        "of the rules in the file" << std::endl;
        
        return 1;
    }

    if (spec_input.empty()) {
        is_spec_available = false;
    } else {
        std::cout << "rfscss - parsing specification provided" << std::endl;
        std::shared_ptr<State> spec_state(new State());
        spec = rfscss_spec::parse_spec(spec_state, spec_input);
        if (spec_state->error->kind != "") {
            spec_state->error->print(spec_path);
            return 1;
        }
    }

    std::shared_ptr<State> state(new State());
    std::unique_ptr<Parser> parser(new Parser(state));

    // Parse the file
    parser->parse_input(file_path, workspace, input);
    if (state->error->kind != "") {
        state->error->print(file_path);
        return 1;
    }

    if (is_spec_available && !build_spec) {
        // Write the refactored output
        // Create a .scss file for every style rule
        bool is_match = false;
        bool selectors_without_match = false;
        int captured_selectors_amount = 0;
        std::vector<std::string> captures; 
        std::vector<std::string> import_paths;
        for (int style_rule_index = 0; style_rule_index < state->selectors.size(); style_rule_index++) {
            std::string style_rule_name = Utils::trim(state->selectors[style_rule_index]);
            std::string content = state->selectors[style_rule_index];
            
            if (state->content[style_rule_index] != "") {
                content += + "{" + state->content[style_rule_index] + "}\n";
            }

            // try for every spec to find the matching dir
            for (int match_index = 0; match_index < spec.match_strings.size(); match_index++) {
                is_match = Wildcard::match(spec.match_strings[match_index], style_rule_name, captures);
                if (is_match) {
                    captured_selectors_amount++;
                    std::string file_path = spec.output_paths[match_index];
                    // Replace ? for its respective capture index
                    if (!captures.empty()) {
                        for (std::string::size_type i = 0; i < file_path.size(); ++i) {
                            if (file_path[i] == '?')
                            {
                                if (options.count("tidy")) {
                                    file_path.replace(i, 1, Utils::tidy(captures[0]));
                                } else {
                                    file_path.replace(i, 1, captures[0]);
                                }
                                
                                if (options.count("warnings")) {
                                    // Copy file path into a new string and tidy it
                                    // If its not a valid path, warn the user
                                    if (!File::is_valid_path(file_path)) {
                                        std::cerr << "rfscss - warning: '" << file_path << "' path could possibly be invalid" << std::endl;
                                    }
                                }

                                i += captures[0].size() - 1;
                                captures.erase(captures.begin());
                            }
                        }
                    }

                    if (options["export-imports"] != "") {
                        // Add the import path to the list of import paths
                        if (std::find(import_paths.begin(), import_paths.end(), file_path) == import_paths.end()) {
                            std::cout << ". added import path " << file_path << std::endl;
                            import_paths.push_back(file_path);
                        }
                    }

                    File::place_in(workspace + "/" + file_path, content);
                    break;
                }
            }

            if (!is_match) {
                selectors_without_match = true;
            }
        }
        
        if (selectors_without_match && options.count("warnings")) {
            std::cerr << "rfscss - warning: there are rules without match in specification, these will be missing.\ndeclare a '%' or '?' rule if you wish to capture all" << std::endl;
        }

        if (options["export-imports"] != "") {
            std::cout << ". exporting @imports at " << options["export-imports"] << std::endl;
            std::stringstream ss;
            for (std::string import_path : import_paths) {
                ss << "@import \"" << import_path << "\";" << std::endl;
            }

            File::place_in(options["export-imports"], ss.str());
        }

        std::cout << "rfscss - captured " << captured_selectors_amount << " of " << state->selectors.size() << " selectors" << std::endl;
    }
    
    if (build_spec) {
        // Write the specification
        std::variant<std::stringstream, std::string> specification_content;
        
        for (int i = 0; i < state->selectors.size(); i++) {
            std::string style_rule_name = state->selectors[i];

            // Trim the selector name
            style_rule_name = Utils::trim(style_rule_name);
            style_rule_name = Utils::replace(style_rule_name, "\n", " ");

            std::get<std::stringstream>(specification_content) << style_rule_name << "\n";
        }

        specification_content = std::get<std::stringstream>(specification_content).str();
        File::place_in(workspace + "/" + options["list"], std::get<std::string>(specification_content));
        std::cout << "rfscss - list saved at " << workspace << "/" << options["list"] << std::endl;
    }

    return 0;
}