#ifndef HSL_TYPEIMPL_STREAMIMPL_H
#define HSL_TYPEIMPL_STREAMIMPL_H

#include <HSL/Types.h>
#include <HSL/TypeImpl/TypeImpl.h>

class ScriptManager;

#define IS_STREAM(value) IsNativeInstance(value, CLASS_STREAM)
#define AS_STREAM(value) ((ObjStream*)AS_OBJECT(value))

class StreamImpl : public TypeImpl {
public:
	StreamImpl(ScriptManager* manager);

	ObjStream* New(void* streamPtr, bool writable);
	static void Dispose(Obj* object);
};

#endif /* HSL_TYPEIMPL_STREAMIMPL_H */
