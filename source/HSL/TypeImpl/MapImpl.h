#ifndef HSL_TYPEIMPL_MAPIMPL_H
#define HSL_TYPEIMPL_MAPIMPL_H

#include <HSL/Types.h>
#include <HSL/TypeImpl/TypeImpl.h>

class ScriptManager;

class MapImpl : public TypeImpl {
public:
	MapImpl(ScriptManager* manager);

	Obj* New();
	static void Dispose(Obj* object);

#ifdef HSL_VM
	static VMValue VM_GetKeys(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_RemoveKey(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_Iterate(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_IteratorValue(int argCount, VMValue* args, VMThread* thread);
#endif
};

#endif /* HSL_TYPEIMPL_MAPIMPL_H */
