#include <iostream>
#include <string>
#include <memory>
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

    std::string file_path = argv[1];
    std::string file_name = file_path;
    std::string file_ext = file_path;
    std::string workspace = file_path;

    std::string::size_type delim_pos = workspace.find_last_of("/");
    if (delim_pos != std::string::npos) {
        workspace = workspace.substr(0, delim_pos);
    }

    // If workspace is equal to argv[1], then the file is in the same directory
    // as the caller
    if (workspace == argv[1]) {
        // Set the workspace to the current directory
        workspace = ".";
    }

    // Get the file name excluding path
    delim_pos = file_name.find_last_of("/");
    if (delim_pos != std::string::npos) {
        file_name = file_name.substr(delim_pos + 1);
    }

    // Get the file extension
    delim_pos = file_ext.find_last_of(".");
    if (delim_pos != std::string::npos) {
        file_ext = file_ext.substr(delim_pos + 1);
    }

    // Open the file
    std::vector<char> input = File::read(file_path);
    // Check if input is empty
    if (input.empty()) {
        return 1;
    }

    // Open the .rfscss specification. Should be available at "workspace"
    struct Specification spec;
    bool is_spec_available = true;
    std::string spec_path = workspace + "/.rfscss";
    std::vector<char> spec_input = File::read(spec_path);
    if (spec_input.empty()) {
        is_spec_available = false;
        std::cout << "Creating a list specification based on the input...\n";
    } else {
        std::cout << "Refactoring based in specification " << spec_path << std::endl;
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

    if (is_spec_available) {
        // Write the refactored output
        // Create a .scss file for every selector
        bool is_match = false;
        bool selectors_without_match = false;
        std::vector<std::string> captures; 
        std::vector<std::string> import_paths;
        for (int selector_index = 0; selector_index < state->selectors.size(); selector_index++) {
            std::string selector_name = state->selectors[selector_index];
            
            std::string content = state->selectors[selector_index];
            if (state->content[selector_index] != "") {
                content += + "{" + state->content[selector_index] + "}\n";
            }

            // try for every spec to find the matching dir
            for (int match_index = 0; match_index < spec.match_strings.size(); match_index++) {
                is_match = Wildcard::match(spec.match_strings[match_index], selector_name, captures);
                if (is_match) {
                    std::string file_path = spec.output_paths[match_index];

                    // Replace ? for its respective capture index
                    if (!captures.empty()) {
                        for (std::string::size_type i = 0; i != file_path.size(); ++i) {
                            if (file_path[i] == '?') {
                                file_path.replace(i, 1, Utils::tidy(captures[0]));
                                i += captures[0].size() - 1;
                                captures.erase(captures.begin());
                            }
                        }
                    }

                    File::place_in(file_path, content);
                    if (std::find(import_paths.begin(), import_paths.end(), file_path) == import_paths.end()) {
                        std::cout << ". added import path " << file_path << std::endl;
                        import_paths.push_back(file_path);
                    }
                    break;
                }
            }

            if (!is_match) {
                selectors_without_match = true;
            }
        }
        
        // write import.scss file

        std::stringstream ss;
        for (std::string import_path : import_paths) {
            ss << "@import \"" << import_path << "\";" << std::endl;
        }

        if (selectors_without_match) {
            std::cout << "rfScss - warning: there are selectors without match in specification, these will be missing.\nmake sure you are specific-enough or declare a '%' or '?' rule" << std::endl;
        }

        std::cout << ". creating imports." << file_ext << " at " << workspace + "/imports." << file_ext << std::endl;
        File::place_in(workspace + "/imports." + file_ext, ss.str());
    } else {
        // Write the specification
        std::stringstream specification_content;
        for (int i = 0; i < state->selectors.size(); i++) {
            std::string selector_name = state->selectors[i];
            std::string tidy_selector_name;

            // Trim the selector name
            selector_name = Utils::trim(selector_name);
            // Remove newlines from selector name
            selector_name = Utils::replace(selector_name, "\n", "");
            tidy_selector_name = Utils::tidy(selector_name);

            std::string selector_folder = workspace + "/" + tidy_selector_name;
            specification_content << selector_name << "->" << selector_folder << std::endl;
        }

        // File::place_in(workspace + "/.rfscss", specification_content.str());
        std::cout << "List specification saved at " << workspace << "/.rfscss\n";
    }

    return 0;
}