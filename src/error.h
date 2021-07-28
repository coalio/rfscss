#pragma once
#include <vector>
#include <string>

class Error {
public:
    std::string kind = "";
    std::string error_message = "";
    int line; 
    int column;
    int at_char;
    
    void print(std::string file_path);   
};