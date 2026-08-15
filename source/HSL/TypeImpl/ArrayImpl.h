#ifndef HSL_TYPEIMPL_ARRAYIMPL_H
#define HSL_TYPEIMPL_ARRAYIMPL_H

#include <HSL/Types.h>
#include <HSL/TypeImpl/TypeImpl.h>

class ScriptManager;

class ArrayImpl : public TypeImpl {
public:
	ArrayImpl(ScriptManager* manager);

	Obj* New();
	Obj* Allocate();
	static Obj* Constructor(VMThread* thread);
	static void Dispose(Obj* object);

#ifdef HSL_VM
	static VMValue VM_Initializer(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_Iterate(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_IteratorValue(int argCount, VMValue* args, VMThread* thread);
#endif
};

#endif /* HSL_TYPEIMPL_ARRAYIMPL_H */
