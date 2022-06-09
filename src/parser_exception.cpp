#include <iostream>
#include "parser_exception.h"
#include "utils.h"

void parser_exception::print(std::string file_path) {
    std::cout << "rfscss - Unable to parse '" << file_path << "'\n";
    std::cout << Utils::rtrim(this->message);
    // (Of course, the last selector is not necessarily the last one in the file,
    // but rather the last one that was being parsed)
    std::cout << " | " << file_path << ":" << this->line << ":" << this->column << "\n";
    std::cout << "[ cursor: " << this->cursor << " ]" << std::endl;
}