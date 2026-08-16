#ifndef HSL_TYPEIMPL_STRINGIMPL_H
#define HSL_TYPEIMPL_STRINGIMPL_H

#include <HSL/Types.h>
#include <HSL/TypeImpl/TypeImpl.h>

class ScriptManager;

class StringImpl : public TypeImpl {
public:
	StringImpl(ScriptManager* manager);

	static Obj* Constructor(VMThread* thread);
	Obj* New(char* chars, size_t length);
	static void Dispose(Obj* object);

#ifdef HSL_VM
	static bool VM_ElementGet(Obj* object, VMValue at, VMValue* result, VMThread* thread);

#ifdef HAVE_NANOPRINTF
	static VMValue VM_Format(int argCount, VMValue* args, VMThread* thread);
#endif
	static VMValue VM_Split(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_CharAt(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_CodepointAt(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_Length(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_Compare(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_IndexOf(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_Contains(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_Substring(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_ToUpperCase(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_ToLowerCase(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_LastIndexOf(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_ParseInteger(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_ParseDecimal(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_GetCodepoints(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_FromCodepoints(int argCount, VMValue* args, VMThread* thread);
#endif
};

#endif /* HSL_TYPEIMPL_STRINGIMPL_H */
