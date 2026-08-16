#include <HSL/ScriptManager.h>
#include <HSL/TypeImpl/FunctionImpl.h>
#include <HSL/TypeImpl/TypeImpl.h>

FunctionImpl::FunctionImpl(ScriptManager* manager) {
	Manager = manager;
	Class = Manager->NewClass("Function");
	Class->NewFn = Constructor;

#ifdef HSL_VM
	Manager->DefineNative(Class, "Bind", VM_Bind);
	Manager->DefineNative(Class, "bind", VM_Bind);
	Manager->DefineNative(Class, "bindArguments", VM_BindArguments);
#endif

	TypeImpl::RegisterClass(manager, Class);

#ifdef HSL_STDLIB
	TypeImpl::ExposeClass(manager, Class);
#endif
}

Obj* FunctionImpl::Constructor(VMThread* thread) {
	throw ScriptException("Cannot directly construct Function!");
	return nullptr;
}

Obj* FunctionImpl::New() {
	ObjFunction* function = (ObjFunction*)Manager->AllocateObject(sizeof(ObjFunction), OBJ_FUNCTION);
	function->Object.Class = Class;
	function->Chunk.Init();
	return (Obj*)function;
}

#ifdef HSL_VM
/***
 * \method Bind
 * \desc Binds a receiver, and optionally arguments, to a method.
 * \param receiver (value): The receiver to bind.
 * \paramOpt ... (varargs): The arguments to bind.
 * \return <ref BoundMethod> Returns a bound method.
 * \ns Function
 */
VMValue FunctionImpl::VM_Bind(int argCount, VMValue* args, VMThread* thread) {
	if (argCount < 1) {
		thread->CheckAtLeastArgCount(argCount, 1);
		return NULL_VAL;
	}

	ObjFunction* function = thread->GetFunction(args, 0);

	if (argCount > 2) {
		if (argCount - 2 > function->Arity) {
			thread->ThrowRuntimeError("Expected at most %d arguments, but received %d.",
				function->Arity + 1,
				argCount - 1);
			return NULL_VAL;
		}
		else if (argCount - 2 < function->MinArity) {
			thread->ThrowRuntimeError("Expected at least %d arguments, but received only %d.",
				function->MinArity + 1,
				argCount - 1);
			return NULL_VAL;
		}
	}

	ObjBoundMethod* bound = thread->Manager->NewBoundMethod(function, args + 1, argCount - 1);
	bound->HasReceiver = true;
	return OBJECT_VAL(bound);
}

/***
 * \method BindArguments
 * \desc Binds arguments to a method.
 * \param ... (varargs): The arguments to bind.
 * \return <ref BoundMethod> Returns a bound method.
 * \ns Function
 */
VMValue FunctionImpl::VM_BindArguments(int argCount, VMValue* args, VMThread* thread) {
	if (argCount < 1) {
		thread->CheckAtLeastArgCount(argCount, 1);
		return NULL_VAL;
	}

	ObjFunction* function = thread->GetFunction(args, 0);

	if (argCount - 1 > function->Arity) {
		thread->ThrowRuntimeError("Expected at most %d arguments, but received %d.",
			function->Arity,
			argCount - 1);
		return NULL_VAL;
	}
	else if (argCount - 1 < function->MinArity) {
		thread->ThrowRuntimeError("Expected at least %d arguments, but received only %d.",
			function->MinArity,
			argCount - 1);
		return NULL_VAL;
	}

	return OBJECT_VAL(thread->Manager->NewBoundMethod(function, args + 1, argCount - 1));
}
#endif
