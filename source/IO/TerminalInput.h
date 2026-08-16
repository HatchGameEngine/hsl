#ifndef IO_TERMINALINPUT_H
#define IO_TERMINALINPUT_H

#include <Includes/Types.h>
#include <iostream>

#define REPL_TRIM_CHARS " \t\r\v\f"

class TerminalInput {
private:
	static bool ReadLine(std::string& line, const char* prompt);

public:
	static std::string GetLine(const char* prompt);
};

#endif /* IO_TERMINALINPUT_H */
