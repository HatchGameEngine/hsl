#ifndef FILESYSTEM_FILE_H
#define FILESYSTEM_FILE_H

#include <IO/Stream.h>
#include <Includes/Standard.h>

class File {
public:
	enum { READ_ACCESS, WRITE_ACCESS, APPEND_ACCESS };

	static Stream* Open(const char* filename, Uint32 access);
	static bool Exists(const char* path);
};

#endif /* FILESYSTEM_FILE_H */
