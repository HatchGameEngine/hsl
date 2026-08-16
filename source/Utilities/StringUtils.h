#ifndef UTILITIES_STRINGUTILS_H
#define UTILITIES_STRINGUTILS_H

#include <Includes/Standard.h>
#include <HSL/Token.h>

#include <string>
#include <vector>

class StringUtils {
public:
	static char* Create(void* src, size_t length);
	static char* Create(std::string src);

	static char* Duplicate(const char* src);

	static bool StartsWith(const char* string, const char* compare);
	static bool StartsWith(std::string string, std::string compare);
	static char* StrCaseStr(const char* haystack, const char* needle);

	static bool WildcardMatch(const char* first, const char* second);

	static bool ToNumber(int* dst, const char* src);
	static bool ToDecimal(double* dst, const char* src);

	static int DecodeUTF8Char(const char* chr, int& numBytes);
	static std::vector<Uint32> GetCodepoints(const char* text);
	static std::string FromCodepoint(Uint32 codepoint);
	static std::string FromCodepoints(std::vector<Uint32> codepoints);
};

#endif /* UTILITIES_STRINGUTILS_H */
