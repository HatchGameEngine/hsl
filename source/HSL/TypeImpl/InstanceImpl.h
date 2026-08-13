#ifndef HSL_TYPEIMPL_INSTANCEMPL_H
#define HSL_TYPEIMPL_INSTANCEMPL_H

#include <HSL/Types.h>
#include <HSL/TypeImpl/TypeImpl.h>

class ScriptManager;

class InstanceImpl : public TypeImpl {
public:
	InstanceImpl(ScriptManager* manager);

	Obj* New(size_t size, ObjType type);
	static void Dispose(Obj* object);
};

#endif /* HSL_TYPEIMPL_INSTANCEMPL_H */
