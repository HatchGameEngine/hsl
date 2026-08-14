#include <HSL/ScriptManager.h>
#include <HSL/TypeImpl/FunctionImpl.h>
#include <HSL/TypeImpl/TypeImpl.h>

FunctionImpl::FunctionImpl(ScriptManager* manager) {
	Manager = manager;
	Class = Manager->NewClass(CLASS_FUNCTION);

#ifdef HSL_VM
	Manager->DefineNative(Class, "bind", VM_Bind);
	Manager->DefineNative(Class, "bindArguments", VM_BindArguments);
#endif

	TypeImpl::RegisterClass(manager, Class);
}

Obj* FunctionImpl::New() {
	ObjFunction* function = (ObjFunction*)Manager->AllocateObject(sizeof(ObjFunction), OBJ_FUNCTION);
	Memory::Track(function, "NewFunction");
	function->Object.Class = Class;
	function->Chunk.Init();
	return (Obj*)function;
}

#ifdef HSL_VM
#define GET_ARG(argIndex, argFunction) (thread->Manager->argFunction(args, argIndex, thread))

VMValue FunctionImpl::VM_Bind(int argCount, VMValue* args, VMThread* thread) {
	if (argCount < 1) {
		ScriptManager::CheckAtLeastArgCount(argCount, 1, thread);
		return NULL_VAL;
	}

	ObjFunction* function = GET_ARG(0, GetFunction);

	if (argCount > 2) {
		if (argCount - 2 > function->Arity) {
			VM_THROW_ERROR("Expected at most %d arguments, but received %d.",
				function->Arity + 1,
				argCount - 1);
			return NULL_VAL;
		}
		else if (argCount - 2 < function->MinArity) {
			VM_THROW_ERROR("Expected at least %d arguments, but received only %d.",
				function->MinArity + 1,
				argCount - 1);
			return NULL_VAL;
		}
	}

	ObjBoundMethod* bound = thread->Manager->NewBoundMethod(function, args + 1, argCount - 1);
	bound->HasReceiver = true;
	return OBJECT_VAL(bound);
}

VMValue FunctionImpl::VM_BindArguments(int argCount, VMValue* args, VMThread* thread) {
	if (argCount < 1) {
		ScriptManager::CheckAtLeastArgCount(argCount, 1, thread);
		return NULL_VAL;
	}

	ObjFunction* function = GET_ARG(0, GetFunction);

	if (argCount - 1 > function->Arity) {
		VM_THROW_ERROR("Expected at most %d arguments, but received %d.",
			function->Arity,
			argCount - 1);
		return NULL_VAL;
	}
	else if (argCount - 1 < function->MinArity) {
		VM_THROW_ERROR("Expected at least %d arguments, but received only %d.",
			function->MinArity,
			argCount - 1);
		return NULL_VAL;
	}

	return OBJECT_VAL(thread->Manager->NewBoundMethod(function, args + 1, argCount - 1));
}
#endif
