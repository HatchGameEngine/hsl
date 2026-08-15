#include <HSL/ScriptManager.h>
#include <HSL/TypeImpl/InstanceImpl.h>
#include <HSL/TypeImpl/StreamImpl.h>
#include <HSL/TypeImpl/TypeImpl.h>
#include <HSL/Types.h>
#include <IO/Stream.h>

/***
* \class Stream
* \desc An abstraction of a continuous sequence of data.
Use <ref Stream.FromResource> or <ref Stream.FromFile> to open a stream.
*/

StreamImpl::StreamImpl(ScriptManager* manager) {
	Manager = manager;
	Class = Manager->NewClass(CLASS_STREAM);
	Class->NewFn = Constructor;

	TypeImpl::RegisterClass(manager, Class);
}

Obj* StreamImpl::Constructor(VMThread* thread) {
	throw ScriptException("Cannot directly construct Stream! Use Stream.FromResource or Stream.FromFile.");
	return nullptr;
}

ObjStream* StreamImpl::New(void* streamPtr, bool writable) {
	ObjStream* stream = (ObjStream*)Manager->NewNativeInstance(sizeof(ObjStream));
	Memory::Track(stream, "NewStream");
	stream->Object.Class = Class;
	stream->InstanceObj.Destructor = Dispose;
	stream->StreamPtr = (Stream*)streamPtr;
	stream->Writable = writable;
	stream->Closed = false;
	return stream;
}

void StreamImpl::Dispose(Obj* object) {
	ObjStream* stream = (ObjStream*)object;

	if (!stream->Closed) {
		stream->StreamPtr->Close();
	}

	InstanceImpl::Dispose(object);
}
