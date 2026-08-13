#include <Main.h>

#if defined(WIN32) && !defined(WINDOWS_CONSOLE_APP) && !defined(_MSC_VER)
int WinMain(int argc, char* argv[]) {
#else
int main(int argc, char* argv[]) {
#endif
	return HSLMain(argc, argv);
}
