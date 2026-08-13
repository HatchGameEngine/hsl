#ifndef HASHING_MURMUR_H
#define HASHING_MURMUR_H

#include <Includes/Standard.h>

class Murmur {
public:
	static Uint32 EncryptString(char* message);
	static Uint32 EncryptString(const char* message);
	static Uint32 EncryptData(const void* data, size_t size);
	static Uint32 EncryptData(const void* key, size_t size, Uint32 hash);
};

#endif /* HASHING_MURMUR_H */
