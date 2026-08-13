#include <Filesystem/Path.h>

std::string Path::ToString(std::filesystem::path path) {
#if __cplusplus >= 202002L
	std::u8string string = path.u8string();
#else
	auto string = path.u8string();
#endif

	return std::string(string.begin(), string.end());
}
