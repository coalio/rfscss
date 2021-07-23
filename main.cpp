// Import the standard libraries
#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include "src/file.h"
#include "src/state.h"
#include "src/parser.h"

int main(int argc, char* argv[])
{
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

    // Create a new parser instance
    Parser * parser;
    // Open the file
    std::vector<char> input = File::read(file_path);
    // Check if input is empty
    if (input.empty()) {
        return 1;
    }

    // Create a new state instance
    struct State * state = new State();

    // Parse the file
    state = parser->parse_input(file_path, workspace, input);

    // Write the refactored output

    // Create a .scss file for every identifier
    for (int i = 0; i < state->identifiers.size(); i++) {
        // Create a folder using the identifier as name
        std::string folder_name = state->identifiers[i];
        std::string folder_path = workspace + "/" + folder_name;
        std::string content = state->content[i];
        File::place_in(folder_path, folder_name, content);
    }

    return 0;
}