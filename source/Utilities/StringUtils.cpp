#include <Diagnostics/Memory.h>
#include <Filesystem/Path.h>
#include <Utilities/StringUtils.h>

char* StringUtils::Create(void* src, size_t length) {
	char* string = (char*)Memory::Malloc(length + 1);
	memcpy(string, src, length);
	string[length] = '\0';
	return string;
}
char* StringUtils::Create(std::string src) {
	return StringUtils::Duplicate(src.c_str());
}

char* StringUtils::Duplicate(const char* src) {
	size_t length = strlen(src) + 1;
	char* string = (char*)Memory::Malloc(length);
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
