#include <HSL/ScriptManager.h>
#include <HSL/TypeImpl/StringImpl.h>
#include <HSL/TypeImpl/TypeImpl.h>
#include <Utilities/StringUtils.h>

#ifdef HAVE_NANOPRINTF
#define NANOPRINTF_IMPLEMENTATION
#define NANOPRINTF_USE_FIELD_WIDTH_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_PRECISION_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_FLOAT_FORMAT_SPECIFIERS 1
#define NANOPRINTF_USE_BINARY_FORMAT_SPECIFIERS 1
#define NANOPRINTF_VISIBILITY_STATIC 1

#include "../third_party/nanoprintf/nanoprintf.h"
#endif

/***
* \class String
* \desc A sequence of characters. All string literals are instances of String.
*/

StringImpl::StringImpl(ScriptManager* manager) {
	Manager = manager;
	Class = Manager->NewClass("String");
	Class->NewFn = Constructor;
#ifdef HSL_VM
	Class->ElementGet = VM_ElementGet;

#ifdef HAVE_NANOPRINTF
	Manager->DefineNative(Class, "Format", VM_Format);
#endif
	Manager->DefineNative(Class, "Split", VM_Split);
	Manager->DefineNative(Class, "CharAt", VM_CharAt);
	Manager->DefineNative(Class, "CodepointAt", VM_CodepointAt);
	Manager->DefineNative(Class, "Length", VM_Length);
	Manager->DefineNative(Class, "Compare", VM_Compare);
	Manager->DefineNative(Class, "IndexOf", VM_IndexOf);
	Manager->DefineNative(Class, "Contains", VM_Contains);
	Manager->DefineNative(Class, "Substring", VM_Substring);
	Manager->DefineNative(Class, "ToUpperCase", VM_ToUpperCase);
	Manager->DefineNative(Class, "ToLowerCase", VM_ToLowerCase);
	Manager->DefineNative(Class, "LastIndexOf", VM_LastIndexOf);
	Manager->DefineNative(Class, "ParseInteger", VM_ParseInteger);
	Manager->DefineNative(Class, "ParseDecimal", VM_ParseDecimal);
	Manager->DefineNative(Class, "GetCodepoints", VM_GetCodepoints);
	Manager->DefineNative(Class, "FromCodepoints", VM_FromCodepoints);
#endif

	TypeImpl::RegisterClass(manager, Class);
	TypeImpl::ExposeClass(manager, Class);
}

Obj* StringImpl::Constructor(VMThread* thread) {
	throw ScriptException("Cannot directly construct String!");
	return nullptr;
}

Obj* StringImpl::New(char* chars, size_t length) {
	ObjString* string = (ObjString*)Manager->AllocateObject(sizeof(ObjString), OBJ_STRING);
	string->Object.Class = Class;
	string->Length = length;
	string->Chars = chars;
	return (Obj*)string;
}

void StringImpl::Dispose(Obj* object) {
	ObjString* string = (ObjString*)object;

	MEMORY_FREE(string->Chars);
}

#ifdef HSL_VM
bool StringImpl::VM_ElementGet(Obj* object, VMValue at, VMValue* result, VMThread* thread) {
	ObjString* string = (ObjString*)object;

	if (!IS_INTEGER(at)) {
		thread->ThrowRuntimeError("Cannot get value from array using non-Integer value as an index.");
		if (result) {
			*result = NULL_VAL;
		}
		return true;
	}

	int index = AS_INTEGER(at);
	if (index < 0) {
		index = string->Length + index;
	}

	if (index >= string->Length) {
		thread->ThrowRuntimeError("Index %d is out of bounds of string of length %d.",
			index,
			(int)string->Length);
		if (result) {
			*result = NULL_VAL;
		}
		return true;
	}

	if (result) {
		*result = OBJECT_VAL(thread->Manager->CopyString(&string->Chars[index], 1));
	}
	return true;
}

#ifdef HAVE_NANOPRINTF
/***
 * String.Format
 * \desc Formats a <b>format string</b> according to the given <b>format specifiers</b>. A format specifier is a string of the form <code>%[flags][width][.precision][conversion specifier]</code> where a <b>conversion specifier</b> must be one of the following:<br/>\
<ul><li>`d`: Integers</li>\
<li>`f` or `%F`: Decimals</li>\
<li>`s`: Strings</li>\
<li>`c`: Characters</li>\
<li>`x` or `%X`: Hexadecimal integers</li>\
<li>`b` or `%b`: Binary integers</li>\
<li>`o`: Octal integers</li>\
<li>`%`: A literal percent sign character</li>\
</ul>\
<b>Flags</b> are optional, and must be one of the following:<br/>\
<ul><li>`0`: Pads the value with leading zeroes. See the <b>width sub-specifier</b>.</li>\
<li>`-`: Left-justifies the result. See the <b>width sub-specifier</b>.</li>\
<li>`#`: Prefixes something to the value depending on the <b>conversion specifier</b>:\
<ul><li>`x` or `X`: Prefixes the value with `0x` or `0X` respectively.</li>\
<li>`b` or `B`: Prefixes the value with `0b` or `0B` respectively.</li>\
<li>`f`: Prefixes the value with a `.` character.</li>\
<li>`o`: Prefixes the value with a `0` character.</li>\
</ul>\
</li>\
<li>`+`: Prefixes positive numbers with a plus sign.</li>\
<li>A space character: If no sign character (`-` or `+`) was written, a space character is written instead.</li>\
</ul>\
A <b>width sub-specifier</b> is used in conjunction with the flags:<br/>\
<ul><li>A number: The amount of padding to add.</li>\
<li>`*`: This functions the same as the above, but the width is given in the next argument as an Integer value.</li>\
</ul>\
<b>Precision specifiers</b> are also supported:<br/>\
<ul><li>`.` followed by a number:<ul><li>For Integer values, this pads the value with leading zeroes.</li>\
<li>For Decimal values, this specifies the number of digits to be printed after the decimal point (which is 6 by default).</li>\
<li>For String values, this is the maximum amount of characters to be printed.</li>\
</ul>\
</li>\
<li>`.` followed by a `*`: This functions the same as the above, but the precision is given in the next argument as an Integer value.</li>\
</ul>
 * \param string (string): The format string.
 * \paramOpt values (value): Variable arguments.
 * \return string Returns a string value.
 * \ns String
 */
VMValue StringImpl::VM_Format(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckAtLeastArgCount(argCount, 1);
	char* fmtString = thread->GetString(args, 0);

	if (fmtString && thread->Manager->Lock()) {
		ObjString* newString;
		if (argCount > 1) {
			newString = npf_vpprintf(fmtString, argCount - 1, args + 1, thread);
		}
		else {
			newString = thread->Manager->CopyString(fmtString);
		}
		thread->Manager->Unlock();
		return OBJECT_VAL(newString);
	}

	return NULL_VAL;
}
#endif

/***
 * String.Split
 * \desc Splits a string by a delimiter.
 * \param string (string): The string to split.
 * \param delimiter (string): The delimiter string.
 * \return array Returns an array of string values.
 * \ns String
 */
VMValue StringImpl::VM_Split(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);
	char* string = thread->GetString(args, 0);
	char* delimt = thread->GetString(args, 1);

	if (thread->Manager->Lock()) {
		ObjArray* array = thread->Manager->NewArray();

		char* input = StringUtils::Duplicate(string);
		char* tok = strtok(input, delimt);
		while (tok != NULL) {
			array->Values->push_back(OBJECT_VAL(thread->Manager->CopyString(tok)));
			tok = strtok(NULL, delimt);
		}
		MEMORY_FREE(input);

		thread->Manager->Unlock();
		return OBJECT_VAL(array);
	}
	return NULL_VAL;
}
/***
 * String.CharAt
 * \desc Gets the 8-bit value of the character at the specified index.
 * \param string (string): The string containing the character.
 * \param index (integer): The character index to check.
 * \return integer Returns the value as an integer.
 * \ns String
 */
VMValue StringImpl::VM_CharAt(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckAtLeastArgCount(argCount, 2);
	char* string = thread->GetString(args, 0);
	int n = thread->GetInteger(args, 1);

	return INTEGER_VAL((Uint8)string[n]);
}
/***
 * String.CodepointAt
 * \desc Gets the codepoint value of the character at the specified index.
 * \param string (string): The string containing the character.
 * \param index (integer): The character index to check.
 * \return integer Returns the value as an integer.
 * \ns String
 */
VMValue StringImpl::VM_CodepointAt(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckAtLeastArgCount(argCount, 2);
	char* string = thread->GetString(args, 0);
	int n = thread->GetInteger(args, 1);

	return INTEGER_VAL(StringUtils::DecodeUTF8Char(string, n));
}
/***
 * String.Length
 * \desc Gets the length of the string value.
 * \param string (string): The input string.
 * \return integer Returns the length of the string value as an integer.
 * \ns String
 */
VMValue StringImpl::VM_Length(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);
	char* string = thread->GetString(args, 0);
	return INTEGER_VAL((int)strlen(string));
}
/***
 * String.Compare
 * \desc Compares two strings lexicographically.
 * \param stringA (string): The first string to compare.
 * \param stringB (string): The second string to compare.
 * \return integer Returns the comparison result as an integer. The return value is a negative integer if <param stringA> appears before <param stringB> lexicographically, a positive integer if <param stringA> appears after <param stringB> lexicographically, and zero if <param stringA> and <param stringB> are equal.
 * \ns String
 */
VMValue StringImpl::VM_Compare(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);
	char* stringA = thread->GetString(args, 0);
	char* stringB = thread->GetString(args, 1);
	return INTEGER_VAL((int)strcmp(stringA, stringB));
}
/***
 * String.IndexOf
 * \desc Get the first index at which the substring occurs in the string.
 * \param string (string): The string to compare.
 * \param substring (string): The substring to search for.
 * \return integer Returns the index as an integer.
 * \ns String
 */
VMValue StringImpl::VM_IndexOf(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);
	char* string = thread->GetString(args, 0);
	char* substring = thread->GetString(args, 1);
	char* find = strstr(string, substring);
	if (!find) {
		return INTEGER_VAL(-1);
	}
	return INTEGER_VAL((int)(find - string));
}
/***
 * String.Contains
 * \desc Searches for whether a substring is within a string value.
 * \param string (string): The string to compare.
 * \param substring (string): The substring to search for.
 * \return boolean Returns a boolean value.
 * \ns String
 */
VMValue StringImpl::VM_Contains(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);
	char* string = thread->GetString(args, 0);
	char* substring = thread->GetString(args, 1);
	return INTEGER_VAL((int)(!!strstr(string, substring)));
}
/***
 * String.Substring
 * \desc Get a string value from a portion of a larger string value.
 * \param string (string): The input string.
 * \param startIndex (integer): The starting index of the substring.
 * \param length (integer): The length of the substring.
 * \return string Returns a string value.
 * \ns String
 */
VMValue StringImpl::VM_Substring(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 3);
	char* string = thread->GetString(args, 0);
	size_t index = thread->GetInteger(args, 1);
	size_t length = thread->GetInteger(args, 2);

	size_t strln = strlen(string);
	if (length > strln - index) {
		length = strln - index;
	}

	VMValue obj = NULL_VAL;
	if (thread->Manager->Lock()) {
		obj = OBJECT_VAL(thread->Manager->CopyString(string + index, length));
		thread->Manager->Unlock();
	}
	return obj;
}
/***
 * String.ToUpperCase
 * \desc Convert a string value to its uppercase representation.
 * \param string (string): The string to make uppercase.
 * \return string Returns a uppercase string value.
 * \ns String
 */
VMValue StringImpl::VM_ToUpperCase(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);
	char* string = thread->GetString(args, 0);
	if (string == nullptr) {
		return NULL_VAL;
	}

	VMValue obj = NULL_VAL;
	if (thread->Manager->Lock()) {
		char* copy = StringUtils::Duplicate(string);
		for (char* a = copy; *a; a++) {
			if (*a >= 'a' && *a <= 'z') {
				*a += 'A' - 'a';
			}
			else if (*a >= 'a' && *a <= 'z') {
				*a += 'A' - 'a';
			}
		}
		obj = OBJECT_VAL(thread->Manager->CopyString(copy));
		thread->Manager->Unlock();
	}
	return obj;
}
/***
 * String.ToLowerCase
 * \desc Convert a string value to its lowercase representation.
 * \param string (string): The string to make lowercase.
 * \return string Returns a lowercase string value.
 * \ns String
 */
VMValue StringImpl::VM_ToLowerCase(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);
	char* string = thread->GetString(args, 0);
	if (string == nullptr) {
		return NULL_VAL;
	}

	VMValue obj = NULL_VAL;
	if (thread->Manager->Lock()) {
		char* copy = StringUtils::Duplicate(string);
		for (char* a = copy; *a; a++) {
			if (*a >= 'A' && *a <= 'Z') {
				*a += 'a' - 'A';
			}
		}
		obj = OBJECT_VAL(thread->Manager->CopyString(copy));
		thread->Manager->Unlock();
	}
	return obj;
}
/***
 * String.LastIndexOf
 * \desc Get the last index at which the substring occurs in the string.
 * \param string (string): The string to compare.
 * \param substring (string): The substring to search for.
 * \return integer Returns the index as an integer.
 * \ns String
 */
VMValue StringImpl::VM_LastIndexOf(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);
	char* string = thread->GetString(args, 0);
	char* substring = thread->GetString(args, 1);
	size_t string_len = strlen(string);
	size_t substring_len = strlen(substring);
	if (string_len < substring_len) {
		return INTEGER_VAL(-1);
	}

	char* find = NULL;
	for (char* start = string + string_len - substring_len; start >= string; start--) {
		if (memcmp(start, substring, substring_len) == 0) {
			find = start;
			break;
		}
	}
	if (!find) {
		return INTEGER_VAL(-1);
	}
	return INTEGER_VAL((int)(find - string));
}
/***
 * String.ParseInteger
 * \desc Converts a string value to an integer value, if possible.
 * \param string (string): The string to parse.
 * \paramOpt radix (integer): The numerical base, or radix. If `0`, the radix is detected by the value of `string`: <br/>\
If <param string> begins with `0x`, it is a hexadecimal number (base 16);<br/>\
Else, if <param string> begins with `0`, it is an octal number (base 8);<br/>\
Else, if <param string> begins with `0b`, it is a binary number (base 2);<br/>\
Else, the number is assumed to be in base 10.
 * \return integer Returns the value as an integer.
 * \ns String
 */
VMValue StringImpl::VM_ParseInteger(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckAtLeastArgCount(argCount, 1);
	char* string = thread->GetString(args, 0);
	int radix = GET_ARG_OPT(1, GetInteger, 10);
	if (radix < 0 || radix > 36) {
		thread->ThrowRuntimeError("Invalid radix of %d. (0 - 36)", radix);
		return NULL_VAL;
	}
	return INTEGER_VAL((int)strtol(string, NULL, radix));
}
/***
 * String.ParseDecimal
 * \desc Convert a string value to a decimal value if possible.
 * \param string (string): The string to parse.
 * \return decimal Returns the value as a decimal.
 * \ns String
 */
VMValue StringImpl::VM_ParseDecimal(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);
	char* string = thread->GetString(args, 0);
	return DECIMAL_VAL((float)strtod(string, NULL));
}
/***
 * String.GetCodepoints
 * \desc Gets a list of UCS codepoints from UTF-8 text.
 * \param string (string): The UTF-8 string.
 * \return array Returns an array of Integer values.
 * \ns String
 */
VMValue StringImpl::VM_GetCodepoints(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);
	char* string = thread->GetString(args, 0);

	ObjArray* array = thread->Manager->NewArray();

	if (string) {
		std::vector<Uint32> codepoints = StringUtils::GetCodepoints(string);

		for (size_t i = 0; i < codepoints.size(); i++) {
			array->Values->push_back(INTEGER_VAL((int)codepoints[i]));
		}
	}

	return OBJECT_VAL(array);
}
/***
 * String.FromCodepoints
 * \desc Creates UTF-8 text from a list of UCS codepoints.
 * \param codepoints (array): An array of integer values.
 * \return string Returns a string value.
 * \ns String
 */
VMValue StringImpl::VM_FromCodepoints(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);
	ObjArray* array = thread->GetArray(args, 0);
	if (!array) {
		return NULL_VAL;
	}

	std::vector<Uint32> codepoints;
	std::string result;

	for (size_t i = 0; i < array->Values->size(); i++) {
		VMValue value = (*array->Values)[i];
		int codepoint = 0;

		if (IS_INTEGER(value)) {
			codepoint = AS_INTEGER(value);
		}
		else {
			thread->ThrowRuntimeError("Expected array index %d to be of type %s instead of %s.",
				i,
				GetTypeString(VAL_INTEGER),
				GetValueTypeString(value));
		}

		codepoints.push_back(codepoint);
	}

	try {
		result = StringUtils::FromCodepoints(codepoints);
	} catch (const std::runtime_error& error) {
		thread->ThrowRuntimeError("%s", error.what());
		return NULL_VAL;
	}

	if (thread->Manager->Lock()) {
		ObjString* string = thread->Manager->CopyString(result);
		thread->Manager->Unlock();
		return OBJECT_VAL(string);
	}
	return NULL_VAL;
}
#endif
