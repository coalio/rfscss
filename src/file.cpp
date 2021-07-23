#include <vector>
#include <string>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include "file.h"

std::vector<char> File::read(std::string file_path) {
    std::vector<char> content;
    std::ifstream file;

    file.open(file_path, std::ios::in);
    if (file.is_open())
    {
        while (!file.eof())
        {
            char c = file.get();
            int c_byte = static_cast<int>(c);
            if (c_byte == -1) break;
            content.push_back(c);
        }
        file.close();
    } else {
        std::cout << "rfScss - File '" << file_path << "' not found" << std::endl;
    }

    return content;
}

void File::place_in(std::string folder_path, std::string folder_name, std::string content) {
    // If the folder does not exist, create it
    struct stat info;
    
    if( stat( folder_path.c_str(), &info ) != 0 ) {
        std::cout << "rfScss - No permissions to access: " << folder_path << std::endl;
    } else if( info.st_mode & S_IFDIR ) {
        std::cout << "rfScss - Reusing folder: %s" << folder_path << std::endl;
    } else {
        std::cout << "rfScss - Creating folder: " << folder_path << std::endl;
        int ret = std::system(("mkdir " + folder_path).c_str());

        if (ret != 0) {
            std::cout << "rfScss - Unable to create folder: " << folder_path << std::endl;
            return;
        }
    }

    // Create a .scss file for this identifier and put the content in it
    std::string file_name = folder_name + ".scss";
    std::string file_path = folder_path + "/" + file_name;
    std::cout << "rfScss - Creating file: " << file_path << std::endl;
    std::stringstream ss;
    ss << "." << folder_name << " {";
    ss << content << "}\n";
    std::ofstream file;
    file.open(file_path, std::ios::out);
    file << ss.str();
    file.close();
}