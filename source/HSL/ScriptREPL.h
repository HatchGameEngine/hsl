#ifndef HSL_SCRIPTREPL_H
#define HSL_SCRIPTREPL_H

#include <HSL/Types.h>
#include <HSL/VMThread.h>

#ifdef HSL_VM
#include <HSL/CompilerEnums.h>
#endif

class ScriptREPL {
#ifdef HSL_VM
public:
	static VMValue ExecuteCode(VMThread* thread, CallFrame* frame, const char* code, CompilerSettings settings);
#endif
};

#endif /* HSL_SCRIPTREPL_H */
