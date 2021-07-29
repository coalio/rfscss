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

int main(int argc, char* argv[])
{
    umask(0);

    std::string file_path = argv[1];
    std::string file_name = file_path;
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
        return 1;
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
        for (int i = 0; i < state->selectors.size(); i++) {
            std::string file_name = state->selectors[i];
            // Trim the folder_name
            file_name = Utils::trim(file_name);
            std::string file_path = workspace + "/" + file_name + ".scss";
            std::string content = state->selectors[i] + "{" + state->content[i] + "}\n";
            File::place_in(file_path, content);
        }
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

        File::place_in(workspace + "/.rfscss", specification_content.str());
        std::cout << "List specification saved at " << workspace << "/.rfscss\n";
    }

    return 0;
}