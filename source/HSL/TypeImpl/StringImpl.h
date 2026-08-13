#ifndef HSL_TYPEIMPL_STRINGIMPL_H
#define HSL_TYPEIMPL_STRINGIMPL_H

#include <HSL/Types.h>
#include <HSL/TypeImpl/TypeImpl.h>

class ScriptManager;

class StringImpl : public TypeImpl {
public:
	StringImpl(ScriptManager* manager);

	Obj* New(char* chars, size_t length);
	static void Dispose(Obj* object);

#ifdef HSL_VM
	static bool VM_ElementGet(Obj* object, VMValue at, VMValue* result, VMThread* thread);
	static bool VM_ElementSet(Obj* object, VMValue at, VMValue value, VMThread* thread);
#endif
};

#endif /* HSL_TYPEIMPL_STRINGIMPL_H */
