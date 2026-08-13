#ifndef FILESYSTEM_PATH_H
#define FILESYSTEM_PATH_H

#include <Includes/Standard.h>

#define MAX_PATH_LENGTH 4096

class Path {
private:
public:
	static std::string ToString(std::filesystem::path path);
};

#endif /* FILESYSTEM_PATH_H */
