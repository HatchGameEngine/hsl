#ifndef HSL_TYPEIMPL_FUNCTIONIMPL_H
#define HSL_TYPEIMPL_FUNCTIONIMPL_H

#include <HSL/Types.h>
#include <HSL/TypeImpl/TypeImpl.h>

class ScriptManager;

class FunctionImpl : public TypeImpl {
public:
	FunctionImpl(ScriptManager* manager);

	static Obj* Constructor(VMThread* thread);
	Obj* New();

#ifdef HSL_VM
	static VMValue VM_Bind(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_BindArguments(int argCount, VMValue* args, VMThread* thread);
#endif
};

#endif /* HSL_TYPEIMPL_FUNCTIONIMPL_H */
