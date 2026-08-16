#include <Utilities/Memory.h>
#include <Utilities/StringUtils.h>

#include <climits>
#include <cstring>
#include <cmath>
#include <stdexcept>

char* StringUtils::Create(void* src, size_t length) {
	char* string = (char*)MEMORY_ALLOC(length + 1);
	memcpy(string, src, length);
	string[length] = '\0';
	return string;
}
char* StringUtils::Create(std::string src) {
	return StringUtils::Duplicate(src.c_str());
}

char* StringUtils::Duplicate(const char* src) {
	size_t length = strlen(src) + 1;
	char* string = (char*)MEMORY_ALLOC(length);
	memcpy(string, src, length);
	return string;
}

bool StringUtils::StartsWith(const char* string, const char* compare) {
	size_t cmpLen = strlen(compare);
	if (strlen(string) < cmpLen) {
		return false;
	}

	return memcmp(string, compare, cmpLen) == 0;
}
bool StringUtils::StartsWith(std::string string, std::string compare) {
	size_t cmpLen = compare.size();
	if (string.size() < cmpLen) {
		return false;
	}

	return memcmp(string.c_str(), compare.c_str(), cmpLen) == 0;
}

char* StringUtils::StrCaseStr(const char* haystack, const char* needle) {
	if (!needle[0]) {
		return (char*)haystack;
	}

	/* Loop over all possible start positions. */
	for (size_t i = 0; haystack[i]; i++) {
		bool matches = true;
		/* See if the string matches here. */
		for (size_t j = 0; needle[j]; j++) {
			/* If we're out of room in the haystack, give
			 * up. */
			if (!haystack[i + j]) {
				return NULL;
			}

			/* If there's a character mismatch, the needle
			 * doesn't fit here. */
			if (tolower((unsigned char)needle[j]) !=
				tolower((unsigned char)haystack[i + j])) {
				matches = false;
				break;
			}
		}
		if (matches) {
			return (char*)(haystack + i);
		}
	}
	return NULL;
}

bool StringUtils::ToNumber(int* dst, const char* src) {
	char* end;
	long num = strtol(src, &end, 10);

	if (*end != '\0') {
		return false;
	}
	else if (num > INT_MAX || (errno == ERANGE && num == LONG_MAX)) {
		return false;
	}
	else if (num < INT_MIN || (errno == ERANGE && num == LONG_MIN)) {
		return false;
	}

	(*dst) = num;
	return true;
}
bool StringUtils::ToDecimal(double* dst, const char* src) {
	char* end;
	double num = strtod(src, &end);

	if (*end != '\0') {
		return false;
	}
	if (errno == ERANGE && (num == HUGE_VAL || num == -HUGE_VAL)) {
		return false;
	}

	(*dst) = num;
	return true;
}

bool StringUtils::WildcardMatch(const char* first, const char* second) {
	if (*first == 0 && *second == 0) {
		return true;
	}
	if (*first == 0 && *second == '*' && *(second + 1) != 0) {
		return false;
	}
	if (*first == *second || *second == '?') {
		return StringUtils::WildcardMatch(first + 1, second + 1);
	}
	if (*second == '*') {
		return StringUtils::WildcardMatch(first, second + 1) ||
			StringUtils::WildcardMatch(first + 1, second);
	}
	return false;
}

int StringUtils::DecodeUTF8Char(const char* chr, int& numBytes) {
	numBytes = 1;

	if (!chr) {
		return 0;
	}

	unsigned char c = *chr;
	if (c <= 0x7F) {
		return c;
	}

	if ((c & 0xE0) == 0xC0) {
		numBytes = 2;

		if ((chr[1] & 0xC0) != 0x80) {
			return -1;
		}

		int decoded = (chr[0] & 0x1F) << 6;
		decoded |= chr[1] & 0x3F;

		if (decoded < 0x80) {
			return -1;
		}

		return decoded;
	}
	else if ((c & 0xF0) == 0xE0) {
		numBytes = 3;

		if ((chr[1] & 0xC0) != 0x80) {
			return -1;
		}
		else if ((chr[2] & 0xC0) != 0x80) {
			return -1;
		}

		int decoded = (chr[0] & 0x0F) << 12;
		decoded |= (chr[1] & 0x3F) << 6;
		decoded |= chr[2] & 0x3F;

		if (decoded < 0x800 || (decoded >= 0xD800 && decoded <= 0xDFFF)) {
			return -1;
		}

		return decoded;
	}
	else if ((c & 0xF8) == 0xF0) {
		numBytes = 4;

		if ((chr[1] & 0xC0) != 0x80) {
			return -1;
		}
		else if ((chr[2] & 0xC0) != 0x80) {
			return -1;
		}
		else if ((chr[3] & 0xC0) != 0x80) {
			return -1;
		}

		int decoded = (chr[0] & 0x07) << 18;
		decoded |= (chr[1] & 0x3F) << 12;
		decoded |= (chr[2] & 0x3F) << 6;
		decoded |= chr[3] & 0x3F;

		if (decoded < 0x10000 || decoded > 0x10FFFF) {
			return -1;
		}

		return decoded;
	}

	return -1;
}

// Handles UTF-8 text and obtains UCS codepoints
std::vector<Uint32> StringUtils::GetCodepoints(const char* text) {
	size_t textLength = strlen(text);

	std::vector<Uint32> codepoints;
	codepoints.reserve(textLength);

	for (size_t i = 0; i < textLength;) {
		int numBytes = 1;
		int decoded = StringUtils::DecodeUTF8Char(&text[i], numBytes);

		codepoints.push_back((Uint32)decoded);

		i += numBytes;
	}

	return codepoints;
}

// Converts an UCS codepoint to an UTF-8 string
// Throws an exception if it encounters an invalid codepoint.
std::string StringUtils::FromCodepoint(Uint32 codepoint) {
	std::string result;
	result.reserve(4);

	if (codepoint <= 0x7F) {
		result += (char)codepoint;
	}
	else if (codepoint <= 0x7FF) {
		result += (char)(0xC0 | (codepoint >> 6));
		result += (char)(0x80 | (codepoint & 0x3F));
	}
	else if (codepoint <= 0xFFFF) {
		result += (char)(0xE0 | (codepoint >> 12));
		result += (char)(0x80 | ((codepoint >> 6) & 0x3F));
		result += (char)(0x80 | (codepoint & 0x3F));
	}
	else if (codepoint <= 0x10FFFF) {
		result += (char)(0xF0 | (codepoint >> 18));
		result += (char)(0x80 | ((codepoint >> 12) & 0x3F));
		result += (char)(0x80 | ((codepoint >> 6) & 0x3F));
		result += (char)(0x80 | (codepoint & 0x3F));
	}
	else {
		throw std::runtime_error("Invalid UCS codepoint encountered!");
	}

	return result;
}

// Returns UTF-8 text from UCS codepoints
// Throws an exception if it encounters an invalid codepoint.
std::string StringUtils::FromCodepoints(std::vector<Uint32> codepoints) {
	size_t numCodepoints = codepoints.size();

	std::string result;
	result.reserve(numCodepoints * 2);

	for (size_t i = 0; i < numCodepoints; i++) {
		result += StringUtils::FromCodepoint(codepoints[i]);
	}

	return result;
}
