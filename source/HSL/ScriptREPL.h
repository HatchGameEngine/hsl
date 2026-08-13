#ifndef HSL_SCRIPTREPL_H
#define HSL_SCRIPTREPL_H

#include <HSL/CompilerEnums.h>
#include <HSL/Types.h>
#include <HSL/VMThread.h>

class ScriptREPL {
#ifdef HSL_VM
public:
	static VMValue ExecuteCode(VMThread* thread, CallFrame* frame, const char* code, CompilerSettings settings);
#endif
};

#endif /* HSL_SCRIPTREPL_H */
