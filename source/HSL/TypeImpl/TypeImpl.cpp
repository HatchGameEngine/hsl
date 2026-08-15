#include <HSL/ScriptManager.h>
#include <HSL/TypeImpl/TypeImpl.h>

void TypeImpl::Init() {}

void TypeImpl::Dispose() {}

void TypeImpl::RegisterClass(ScriptManager* manager, ObjClass* klass) {
#ifdef HSL_VM
	manager->ImplClasses->Put(klass->Hash, klass);
#endif
}

void TypeImpl::ExposeClass(ScriptManager* manager, ObjClass* klass) {
#ifdef HSL_VM
	manager->Globals->Put(klass->Name, OBJECT_VAL(klass));
#endif
}
