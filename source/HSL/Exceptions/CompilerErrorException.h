#ifndef EXCEPTIONS_COMPILERERROREXCEPTION_H
#define EXCEPTIONS_COMPILERERROREXCEPTION_H

#include <stdexcept>
#include <string>

class CompilerErrorException : public std::runtime_error {
public:
	CompilerErrorException(const std::string& message) : std::runtime_error(message) {}
};

#endif /* HSL_EXCEPTIONS_COMPILERERROREXCEPTION_H */
