#include <Filesystem/Directory.h>

#if WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

bool Directory::Exists(const char* path) {
#if WIN32
	DWORD ftyp = GetFileAttributesA(path);
	if (ftyp == INVALID_FILE_ATTRIBUTES) {
		return false; // Something is wrong with your path
	}
	if (ftyp & FILE_ATTRIBUTE_DIRECTORY) {
		return true;
	}
#else
	struct stat pathStat;
	if (stat(path, &pathStat) == 0) {
		return S_ISDIR(pathStat.st_mode);
	}
#endif
	return false;
}
