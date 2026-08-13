#ifndef HASHING_FNV1A_H
#define HASHING_FNV1A_H

#include <Includes/Standard.h>

class FNV1A {
public:
	static Uint32 EncryptString(char* message);
	static Uint32 EncryptString(const char* message);
	static Uint32 EncryptData(const void* data, size_t size);
	static Uint32 EncryptData(const void* data, size_t size, Uint32 hash);
};

#endif /* HASHING_FNV1A_H */
