#pragma once
#include <exception>
#include <string>

// parser_exception
// This exception is thrown when the parser finds an error
// It contains the line and column of the error
// It also contains the error message, as well as the kind of error
class parser_exception : public std::exception {
public:
    parser_exception(
        int _line,
        int _column,
        int _cursor,
        std::string _message,
        std::string _kind
    ) :
        line(_line),
        column(_column),
        cursor(_cursor),
        message(_message),
        kind(_kind)
    {}

    int line;
    int column;
    int cursor;
    std::string message;
    std::string kind;

    void print(std::string file_path);
};
