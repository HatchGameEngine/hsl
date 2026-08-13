#ifndef FILESYSTEM_DIRECTORY_H
#define FILESYSTEM_DIRECTORY_H

#include <Includes/Standard.h>

class Directory {
public:
	static bool Exists(const char* path);
};

#endif /* FILESYSTEM_DIRECTORY_H */
