#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

class File {
public:
    // Method that reads a file and returns a char vector with the file content
    static std::vector<char> read(std::string file_path);
    static void place_in(
        std::string folder_path, std::string folder_name, std::string content
    );
};