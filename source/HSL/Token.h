#ifndef TOKEN_H
#define TOKEN_H

#include <string>

struct Token {
	int Type;
	char* Start;
	size_t Length;
	int Line;
	size_t Pos;

	std::string ToString() { return std::string(Start, Length); }
};

#endif /* HSL_TOKEN_H */
