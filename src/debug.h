#pragma once
#include <iostream>
#define DEBUG true
#define PRINT_FILE_NAME false
#if DEBUG
// LOG is a debug log macro. It prints the message in the first parameter to cout. Also prints the current file && line.
#if PRINT_FILE_NAME
#define LOG(x) std::cout << "[" << __FILE__ << ":" << __LINE__ << "] " << x << std::endl
#else
#define LOG(x) std::cout << "[" << __LINE__ << "] " << x << std::endl
#endif
#else
// LOG is a debug log macro that is intended to be turned on during
// development.
//
// It is currently disabled. Activate it by defining the DEBUG macro to "true" at debug.h
#define LOG(x)
#endif