#include <iostream>
#include "error.h"
#include "utils.h"

void Error::print(std::string file_path) {
    std::cout << "rfScss - Unable to refactor file: " << file_path << std::endl; 
    std::cout << Utils::rtrim(this->error_message);
    // (Of course, the last selector is not necessarily the last one in the file,
    // but rather the last one that was being parsed)
    std::cout << " | " << file_path << ":" << this->line << ":" << this->column;
    printf("\n[ line: %d, cursor: %d ]\n", this->line, this->at_char);
}