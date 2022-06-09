#pragma once
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

class File {
public:
    // Check if the path is valid under the most common file systems
    static bool is_valid_path(const std::string& path);

    // Reads a file and returns it as a std::string
    static std::string read(std::string file_path);

    // Appends a string to a file
    static void place_in(
        std::string file_path, std::string content
    );
};