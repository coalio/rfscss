#pragma once
#define DEBUG true
#if DEBUG
// LOG is a debug log macro. It prints the message in the first parameter to cout. Also prints the current file and line.
#define LOG(x) std::cout << "[" << __FILE__ << ":" << __LINE__ << "] " << x << std::endl
#else
// LOG is a debug log macro that is intended to be turned on during
// development.
//
// It is currently disabled. Activate it by defining the DEBUG macro to "true" at debug.h
#define LOG(x)
#endif