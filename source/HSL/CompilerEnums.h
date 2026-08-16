#ifndef COMPILER_ENUMS_H
#define COMPILER_ENUMS_H

#include <HSL/Types.h>

struct CompilerSettings {
	bool PrintToLog;
	bool ShowWarnings;
	bool WriteDebugInfo;
	bool WriteSourceFilename;
	bool DoOptimizations;
	bool PrintChunks;
};

#endif /* COMPILER_ENUMS_H */
