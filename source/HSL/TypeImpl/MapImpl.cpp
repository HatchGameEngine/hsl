#include <HSL/ScriptManager.h>
#include <HSL/TypeImpl/MapImpl.h>
#include <HSL/TypeImpl/TypeImpl.h>
#include <HSL/Value.h>

/***
* \class Map
* \desc An associative array, also known as a dictionary, or a map.
*/

MapImpl::MapImpl(ScriptManager* manager) {
	Manager = manager;
	Class = Manager->NewClass("Map");
	Class->NewFn = Constructor;

#ifdef HSL_VM
	Manager->DefineNative(Class, "Length", MapImpl::VM_Length);
	Manager->DefineNative(Class, "GetKeys", MapImpl::VM_GetKeys);
	Manager->DefineNative(Class, "keys", VM_GetKeys);
	Manager->DefineNative(Class, "Remove", VM_Remove);
	Manager->DefineNative(Class, "remove", VM_Remove);
	Manager->DefineNative(Class, "Clear", VM_Clear);
	Manager->DefineNative(Class, "iterate", VM_Iterate);
	Manager->DefineNative(Class, "iteratorValue", VM_IteratorValue);
#endif

	TypeImpl::RegisterClass(manager, Class);
}

Obj* MapImpl::Allocate() {
	ObjMap* map = (ObjMap*)Manager->AllocateObject(sizeof(ObjMap), OBJ_MAP);
	Memory::Track(map, "NewMap");
	map->Object.Class = Class;
	map->Values = new OrderedHashMap<VMValue>(NULL, 4);
	map->Keys = new OrderedHashMap<VMValue>(NULL, 4);
	return (Obj*)map;
}

/***
 * \constructor
 * \desc Creates a map.
 * \ns Map
 */
Obj* MapImpl::Constructor(VMThread* thread) {
	return thread->Manager->ImplMap->Allocate();
}

void MapImpl::Dispose(Obj* object) {
	ObjMap* map = (ObjMap*)object;

	// Free Keys table
	delete map->Keys;

	// Free Values table
	delete map->Values;
}

#ifdef HSL_VM
#define GET_ARG(argIndex, argFunction) (thread->Manager->argFunction(args, argIndex, thread))

/***
 * \method Length
 * \desc Get the number of items in the map.
 * \return integer Returns an integer value.
 * \ns Map
 */
VMValue MapImpl::VM_Length(int argCount, VMValue* args, VMThread* thread) {
	ScriptManager::CheckArgCount(argCount, 1, thread);

	ObjMap* map = GET_ARG(0, GetMap);

	return INTEGER_VAL((int)map->Keys->Count());
}

/***
 * \method GetKeys
 * \desc Gets a list of all keys in the map.
 * \return array Returns an array of values.
 * \ns Map
 */
VMValue MapImpl::VM_GetKeys(int argCount, VMValue* args, VMThread* thread) {
	ScriptManager::CheckArgCount(argCount, 1, thread);

	ObjMap* map = GET_ARG(0, GetMap);

	ObjArray* array = thread->Manager->NewArray();

	map->Keys->WithAllOrdered([array](Uint32, VMValue value) -> void {
		array->Values->push_back(value);
	});

	return OBJECT_VAL(array);
}

/***
 * \method Remove
 * \desc Removes a key from the map.
 * \param key (value): The key to remove.
 * \ns Map
 */
VMValue MapImpl::VM_Remove(int argCount, VMValue* args, VMThread* thread) {
	ScriptManager::CheckArgCount(argCount, 2, thread);

	ObjMap* map = GET_ARG(0, GetMap);
	Uint32 hash = Value::Hash(args[1]);
	map->Keys->Remove(hash);
	map->Values->Remove(hash);

	return NULL_VAL;
}

/***
 * \method Clear
 * \desc Clears the map.
 * \ns Map
 */
VMValue MapImpl::VM_Clear(int argCount, VMValue* args, VMThread* thread) {
	ScriptManager::CheckArgCount(argCount, 1, thread);

	ObjMap* map = GET_ARG(0, GetMap);

	map->Keys->Clear();
	map->Values->Clear();

	return NULL_VAL;
}

VMValue MapImpl::VM_Iterate(int argCount, VMValue* args, VMThread* thread) {
	ScriptManager::CheckArgCount(argCount, 2, thread);

	ObjMap* map = GET_ARG(0, GetMap);

	int key;
	if (IS_NULL(args[1])) {
		key = map->Values->GetFirstKey();
	}
	else {
		key = map->Values->GetNextKey(GET_ARG(1, GetInteger));
	}

	if (key != 0xFFFFFFFF) {
		return INTEGER_VAL(key);
	}

	return NULL_VAL;
}

VMValue MapImpl::VM_IteratorValue(int argCount, VMValue* args, VMThread* thread) {
	ScriptManager::CheckArgCount(argCount, 2, thread);

	ObjMap* map = GET_ARG(0, GetMap);
	int key = GET_ARG(1, GetInteger);

	VMValue value;
	if (map->Values->GetIfExists(key, &value)) {
		return value;
	}

	return NULL_VAL;
}
#endif
