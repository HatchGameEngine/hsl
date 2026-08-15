#include <HSL/ScriptManager.h>
#include <HSL/TypeImpl/ArrayImpl.h>
#include <HSL/TypeImpl/TypeImpl.h>

/***
* \class Array
* \desc A resizable list of values.
*/

ArrayImpl::ArrayImpl(ScriptManager* manager) {
	Manager = manager;
	Class = Manager->NewClass("Array");
	Class->NewFn = Constructor;

#ifdef HSL_VM
	Class->Initializer = OBJECT_VAL(manager->NewNative(VM_Initializer));

	Manager->DefineNative(Class, "iterate", VM_Iterate);
	Manager->DefineNative(Class, "iteratorValue", VM_IteratorValue);
#endif

	TypeImpl::RegisterClass(manager, Class);
	TypeImpl::ExposeClass(manager, Class);
}

Obj* ArrayImpl::Allocate() {
	ObjArray* array = (ObjArray*)Manager->AllocateObject(sizeof(ObjArray), OBJ_ARRAY);
	Memory::Track(array, "NewArray");
	array->Object.Class = Class;
	array->Values = new vector<VMValue>();
	return (Obj*)array;
}

Obj* ArrayImpl::Constructor(VMThread* thread) {
	return thread->Manager->ImplArray->Allocate();
}

#define GET_ARG(argIndex, argFunction) (thread->Manager->argFunction(args, argIndex, thread))

#ifdef HSL_VM
/***
 * \constructor
 * \desc Creates an array.
 * \paramOpt size (integer): Size of the array.
 * \paramOpt initialValue (value): Initial value to set the array elements to.
 * \ns Array
 */
VMValue ArrayImpl::VM_Initializer(int argCount, VMValue* args, VMThread* thread) {
	ObjArray* array = AS_ARRAY(args[0]);

	ScriptManager::CheckAtLeastArgCount(argCount, 1, thread);

	int length = 0;
	VMValue initialValue = NULL_VAL;
	if (argCount >= 2) {
		length = GET_ARG(1, GetInteger);
	}
	if (argCount >= 3) {
		initialValue = args[2];
	}

	for (int i = 0; i < length; i++) {
		array->Values->push_back(initialValue);
	}

	return OBJECT_VAL(array);
}
#endif

void ArrayImpl::Dispose(Obj* object) {
	ObjArray* array = (ObjArray*)object;

	// An array does not own its values, so it's not allowed to free them.
	array->Values->clear();

	delete array->Values;
}

#ifdef HSL_VM
VMValue ArrayImpl::VM_Iterate(int argCount, VMValue* args, VMThread* thread) {
	ScriptManager::CheckArgCount(argCount, 2, thread);

	ObjArray* array = GET_ARG(0, GetArray);

	if (array->Values->size() && IS_NULL(args[1])) {
		return INTEGER_VAL(0);
	}
	else if (!IS_NULL(args[1])) {
		int iteration = GET_ARG(1, GetInteger) + 1;
		if (iteration >= 0 && iteration < array->Values->size()) {
			return INTEGER_VAL(iteration);
		}
	}

	return NULL_VAL;
}

VMValue ArrayImpl::VM_IteratorValue(int argCount, VMValue* args, VMThread* thread) {
	ScriptManager::CheckArgCount(argCount, 2, thread);

	ObjArray* array = GET_ARG(0, GetArray);
	int index = GET_ARG(1, GetInteger);
	if (index < 0 || (Uint32)index >= array->Values->size()) {
		thread->ThrowRuntimeError(false,
			"Index %d is out of bounds of array of size %d.",
			index,
			(int)array->Values->size());
		return NULL_VAL;
	}

	return (*array->Values)[index];
}
#endif
