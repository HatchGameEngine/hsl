#include <HSL/ScriptManager.h>
#include <HSL/TypeImpl/ArrayImpl.h>
#include <HSL/TypeImpl/TypeImpl.h>
#include <HSL/Value.h>

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

	Manager->DefineNative(Class, "Create", VM_Create);
	Manager->DefineNative(Class, "Length", VM_Length);
	Manager->DefineNative(Class, "Push", VM_Push);
	Manager->DefineNative(Class, "Pop", VM_Pop);
	Manager->DefineNative(Class, "Insert", VM_Insert);
	Manager->DefineNative(Class, "Erase", VM_Erase);
	Manager->DefineNative(Class, "Clear", VM_Clear);
	Manager->DefineNative(Class, "Shift", VM_Shift);
	Manager->DefineNative(Class, "SetAll", VM_SetAll);
	Manager->DefineNative(Class, "Reverse", VM_Reverse);
	Manager->DefineNative(Class, "Sort", VM_Sort);
	Manager->DefineNative(Class, "iterate", VM_Iterate);
	Manager->DefineNative(Class, "iteratorValue", VM_IteratorValue);
#endif

	TypeImpl::RegisterClass(manager, Class);
	TypeImpl::ExposeClass(manager, Class);
}

Obj* ArrayImpl::Allocate() {
	ObjArray* array = (ObjArray*)Manager->AllocateObject(sizeof(ObjArray), OBJ_ARRAY);
	array->Object.Class = Class;
	array->Values = new vector<VMValue>();
	return (Obj*)array;
}

Obj* ArrayImpl::Constructor(VMThread* thread) {
	return thread->Manager->ImplArray->Allocate();
}

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

	thread->CheckAtLeastArgCount(argCount, 1);

	int length = 0;
	VMValue initialValue = NULL_VAL;
	if (argCount >= 2) {
		length = thread->GetInteger(args, 1);
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
/***
 * Array.Create
 * \desc Creates an array.
 * \param size (integer): Size of the array.
 * \paramOpt initialValue (value): Initial value to set the array elements to.
 * \return array Returns the created array.
 * \ns Array
 */
VMValue ArrayImpl::VM_Create(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckAtLeastArgCount(argCount, 1);

	if (thread->Manager->Lock()) {
		ObjArray* array = thread->Manager->NewArray();
		int length = thread->GetInteger(args, 0);
		VMValue initialValue = NULL_VAL;
		if (argCount == 2) {
			initialValue = args[1];
		}

		for (int i = 0; i < length; i++) {
			array->Values->push_back(initialValue);
		}

		thread->Manager->Unlock();
		return OBJECT_VAL(array);
	}
	return NULL_VAL;
}
/***
 * Array.Length
 * \desc Gets the length of an array.
 * \param array (array): Array to get the length of.
 * \return integer Length of the array.
 * \ns Array
 */
VMValue ArrayImpl::VM_Length(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);

	if (thread->Manager->Lock()) {
		ObjArray* array = thread->GetArray(args, 0);
		int size = (int)array->Values->size();
		thread->Manager->Unlock();
		return INTEGER_VAL(size);
	}
	return INTEGER_VAL(0);
}
/***
 * Array.Push
 * \desc Adds a value to the end of an array.
 * \param array (array): Array to get the length of.
 * \param value (value): Value to add to the array.
 * \ns Array
 */
VMValue ArrayImpl::VM_Push(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);

	if (thread->Manager->Lock()) {
		ObjArray* array = thread->GetArray(args, 0);
		array->Values->push_back(args[1]);
		thread->Manager->Unlock();
	}
	return NULL_VAL;
}
/***
 * Array.Pop
 * \desc Gets the value at the end of an array, and removes it.
 * \param array (array): Array to get the length of.
 * \return value The value from the end of the array.
 * \ns Array
 */
VMValue ArrayImpl::VM_Pop(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);

	if (thread->Manager->Lock()) {
		ObjArray* array = thread->GetArray(args, 0);
		if (array->Values->size() == 0) {
			thread->Manager->Unlock();
			thread->ThrowRuntimeError("Array is empty!");
			return NULL_VAL;
		}
		VMValue value = array->Values->back();
		array->Values->pop_back();
		thread->Manager->Unlock();
		return value;
	}
	return NULL_VAL;
}
/***
 * Array.Insert
 * \desc Inserts a value at an index of an array.
 * \param array (array): Array to insert value.
 * \param index (integer): Index to insert value.
 * \param value (value): Value to insert.
 * \ns Array
 */
VMValue ArrayImpl::VM_Insert(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 3);

	if (thread->Manager->Lock()) {
		ObjArray* array = thread->GetArray(args, 0);
		int index = thread->GetInteger(args, 1);
		if (index < 0 || index > (int)array->Values->size()) { // Not a typo
			thread->Manager->Unlock();
			thread->ThrowRuntimeError("Index %d is out of bounds of array of size %d.",
				index,
				(int)array->Values->size());
			return NULL_VAL;
		}
		array->Values->insert(array->Values->begin() + index, args[2]);
		thread->Manager->Unlock();
	}
	return NULL_VAL;
}
/***
 * Array.Erase
 * \desc Erases a value at an index of an array.
 * \param array (array): Array to erase value.
 * \param index (integer): Index to erase value.
 * \ns Array
 */
VMValue ArrayImpl::VM_Erase(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);

	if (thread->Manager->Lock()) {
		ObjArray* array = thread->GetArray(args, 0);
		int index = thread->GetInteger(args, 1);
		if (index < 0 || index >= (int)array->Values->size()) {
			thread->Manager->Unlock();
			thread->ThrowRuntimeError("Index %d is out of bounds of array of size %d.",
				index,
				(int)array->Values->size());
			return NULL_VAL;
		}
		array->Values->erase(array->Values->begin() + index);
		thread->Manager->Unlock();
	}
	return NULL_VAL;
}
/***
 * Array.Clear
 * \desc Clears an array.
 * \param array (array): Array to clear.
 * \ns Array
 */
VMValue ArrayImpl::VM_Clear(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);

	if (thread->Manager->Lock()) {
		ObjArray* array = thread->GetArray(args, 0);
		array->Values->clear();
		thread->Manager->Unlock();
	}
	return NULL_VAL;
}
/***
 * Array.Shift
 * \desc Rotates the array in the desired direction.
 * \param array (array): Array to shift.
 * \param toRight (boolean): Whether to rotate the array to the right or not.
 * \ns Array
 */
VMValue ArrayImpl::VM_Shift(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);
	if (thread->Manager->Lock()) {
		ObjArray* array = thread->GetArray(args, 0);
		int toright = thread->GetInteger(args, 1);

		if (array->Values->size() > 1) {
			if (toright) {
				size_t lastIndex = array->Values->size() - 1;
				VMValue temp = (*array->Values)[lastIndex];
				array->Values->erase(array->Values->begin() + lastIndex);
				array->Values->insert(array->Values->begin(), temp);
			}
			else {
				VMValue temp = (*array->Values)[0];
				array->Values->erase(array->Values->begin() + 0);
				array->Values->push_back(temp);
			}
		}

		thread->Manager->Unlock();
	}
	return NULL_VAL;
}
/***
 * Array.SetAll
 * \desc Sets values in the array from startIndex to endIndex (includes the value at endIndex.)
 * \param array (array): Array to set values to.
 * \param startIndex (integer): Index of value to start setting. (`-1` for first index)
 * \param endIndex (integer): Index of value to end setting. (`-1` for last index)
 * \param value (value): Value to set to.
 * \ns Array
 */
VMValue ArrayImpl::VM_SetAll(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 4);
	if (thread->Manager->Lock()) {
		ObjArray* array = thread->GetArray(args, 0);
		size_t startIndex = thread->GetInteger(args, 1);
		size_t endIndex = thread->GetInteger(args, 2);
		VMValue value = args[3];

		size_t arraySize = array->Values->size();
		if (arraySize > 0) {
			if (startIndex < 0) {
				startIndex = 0;
			}
			else if (startIndex >= arraySize) {
				startIndex = arraySize - 1;
			}

			if (endIndex < 0) {
				endIndex = arraySize - 1;
			}
			else if (endIndex >= arraySize) {
				endIndex = arraySize - 1;
			}

			for (size_t i = startIndex; i <= endIndex; i++) {
				(*array->Values)[i] = value;
			}
		}

		thread->Manager->Unlock();
	}
	return NULL_VAL;
}
/***
 * Array.Reverse
 * \desc Reverses the elements of an array through the specified range, exclusive. The array is reversed from `startIndex` to, but not including, `endIndex`.
 * \param array (array): Array to reverse.
 * \paramOpt startIndex (integer): Start range. (default: `0`)
 * \paramOpt endIndex (integer): End range. (default: size of array)
 * \ns Array
 */
VMValue ArrayImpl::VM_Reverse(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckAtLeastArgCount(argCount, 1);
	if (thread->Manager->Lock()) {
		ObjArray* array = thread->GetArray(args, 0);
		int startIndex = GET_ARG_OPT(1, GetInteger, 0);
		int endIndex = GET_ARG_OPT(2, GetInteger, array->Values->size());

		if (startIndex < 0 || startIndex >= (int)array->Values->size() ||
			startIndex >= endIndex) {
			thread->ThrowRuntimeError("Start index out of range.");
			return NULL_VAL;
		}
		if (endIndex <= 0 || endIndex > (int)array->Values->size() ||
			endIndex <= startIndex) {
			thread->ThrowRuntimeError("End index out of range.");
			return NULL_VAL;
		}

		std::reverse(
			array->Values->begin() + startIndex, array->Values->begin() + endIndex);

		thread->Manager->Unlock();
	}
	return NULL_VAL;
}
/***
 * Array.Sort
 * \desc Sorts the entries of the given array.
 * \param array (array): Array to sort.
 * \paramOpt compFunction (function): Comparison function. If not given, a default comparison function is used; the entries of the array are sorted in ascending order, and non-numeric values do not participate in the comparison.
 * \ns Array
 */
VMValue ArrayImpl::VM_Sort(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckAtLeastArgCount(argCount, 1);
	if (thread->Manager->Lock()) {
		ObjArray* array = thread->GetArray(args, 0);
		ObjFunction* function = GET_ARG_OPT(1, GetFunction, nullptr);

		if (function) {
			std::stable_sort(array->Values->begin(),
				array->Values->end(),
				[array, thread, function](const VMValue& a, const VMValue& b) {
					thread->Push(a);
					thread->Push(b);

					VMValue result = thread->RunEntityFunction(function, 2);

					thread->Pop(2);

					if (IS_INTEGER(result)) {
						return AS_INTEGER(result) == 1;
					}

					return false;
				});
		}
		else {
			std::stable_sort(array->Values->begin(),
				array->Values->end(),
				[array](const VMValue& a, const VMValue& b) {
					if (IS_NOT_NUMBER(a) || IS_NOT_NUMBER(b)) {
						return false;
					}
					else if (IS_DECIMAL(a) || IS_DECIMAL(b)) {
						return AS_DECIMAL(Value::CastAsDecimal(a)) <
							AS_DECIMAL(Value::CastAsDecimal(b));
					}
					else {
						return AS_INTEGER(a) < AS_INTEGER(b);
					}
				});
		}

		thread->Manager->Unlock();
	}
	return NULL_VAL;
}

VMValue ArrayImpl::VM_Iterate(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);

	ObjArray* array = thread->GetArray(args, 0);

	if (array->Values->size() && IS_NULL(args[1])) {
		return INTEGER_VAL(0);
	}
	else if (!IS_NULL(args[1])) {
		int iteration = thread->GetInteger(args, 1) + 1;
		if (iteration >= 0 && iteration < array->Values->size()) {
			return INTEGER_VAL(iteration);
		}
	}

	return NULL_VAL;
}

VMValue ArrayImpl::VM_IteratorValue(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);

	ObjArray* array = thread->GetArray(args, 0);
	int index = thread->GetInteger(args, 1);
	if (index < 0 || (Uint32)index >= array->Values->size()) {
		thread->ThrowRuntimeError("Index %d is out of bounds of array of size %d.",
			index,
			(int)array->Values->size());
		return NULL_VAL;
	}

	return (*array->Values)[index];
}
#endif
