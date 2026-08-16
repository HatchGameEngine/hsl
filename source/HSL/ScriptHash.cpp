#include <Hashing/Murmur.h>
#include <HSL/ScriptHash.h>

Uint32 ScriptHash::EncryptString(char* message) {
	return Murmur::EncryptString(message);
}
Uint32 ScriptHash::EncryptString(const char* message) {
	return Murmur::EncryptString((char*)message);
}

Uint32 ScriptHash::EncryptData(const void* data, size_t size) {
	return Murmur::EncryptData(data, size);
}
