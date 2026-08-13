#include <Diagnostics/Log.h>
#include <Error.h>
#include <Includes/Standard.h>

#include <Main.h>

void Error::ShowFatal(const char* errorString, bool showMessageBox) {
	Log::Print(Log::LOG_FATAL, "%s", errorString);

#if HSL_LIBRARY
	abort();
#else
	StandaloneExit(errorString);
#endif
}

void Error::Fatal(const char* errorMessage, ...) {
	va_list args;
	char errorString[2048];
	va_start(args, errorMessage);
	vsnprintf(errorString, sizeof(errorString), errorMessage, args);
	va_end(args);

	ShowFatal(errorString, true);
}
void Error::FatalNoMessageBox(const char* errorMessage, ...) {
	va_list args;
	char errorString[2048];
	va_start(args, errorMessage);
	vsnprintf(errorString, sizeof(errorString), errorMessage, args);
	va_end(args);

	ShowFatal(errorString, false);
}
