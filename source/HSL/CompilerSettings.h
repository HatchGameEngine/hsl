#ifndef HSL_COMPILERSETTINGS_H
#define HSL_COMPILERSETTINGS_H

struct CompilerSettings {
	bool PrintToLog;
	bool ShowWarnings;
	bool WriteDebugInfo;
	bool WriteSourceFilename;
	bool DoOptimizations;
	bool PrintChunks;
};

#endif /* HSL_COMPILERSETTINGS_H */
