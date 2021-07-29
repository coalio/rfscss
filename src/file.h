#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

class File {
public:
    // Method that reads a file && returns a char vector with the file content
    static std::vector<char> read(std::string file_path);
    static void place_in(
        std::string file_path, std::string content
    );
};