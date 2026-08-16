#ifndef HSL_SCRIPTHASH_H
#define HSL_SCRIPTHASH_H

#include <Includes/Types.h>

class ScriptHash {
public:
	static Uint32 EncryptString(char* message);
	static Uint32 EncryptString(const char* message);
	static Uint32 EncryptData(const void* data, size_t size);
};

#endif /* HSL_SCRIPTHASH_H */
