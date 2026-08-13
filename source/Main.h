#ifndef MAIN_H
#define MAIN_H

#include <string>

#include <HSL/API.h>

#ifndef HSL_LIBRARY
int HSLMain(int argc, char* args[]);
#endif

bool InStandaloneREPL();

void InitSubsystems();
void DisposeSubsystems();

bool LockScriptManager(void* manager);
bool UnlockScriptManager(void* manager);

void SetScriptManagerLockFunction(hsl_LockFunction function);
void SetScriptManagerUnlockFunction(hsl_UnlockFunction function);

void StandaloneExit(std::string error);

#ifdef HSL_LIBRARY
class ScriptManager;

hsl_ErrorResponse HandleVMRuntimeError(ScriptManager* manager, hsl_Result error, std::string text);
#endif

bool ShouldShowGarbageCollectionOutput();
const char* GetScriptsDirectory();
void SetScriptsDirectory(const char* directory);
const char* GetVersionText();

#endif /* MAIN_H */
