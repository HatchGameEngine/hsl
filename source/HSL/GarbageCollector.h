#ifndef HSL_GARBAGECOLLECTOR_H
#define HSL_GARBAGECOLLECTOR_H

#include <HSL/Types.h>

#include <set>

class ScriptManager;

class GarbageCollector {
private:
	void FreeObject(Obj* object);
	void GrayValue(VMValue value);
	void BlackenObject(Obj* object);

public:
	std::vector<Obj*> GrayList;
	ScriptManager* Manager;
	size_t NextGC;
	size_t GarbageSize;

	GarbageCollector(ScriptManager* manager);
	void Collect(bool doLog);
	void GrayObject(void* obj);
	void GrayHashMap(void* pointer);
};

#endif /* HSL_GARBAGECOLLECTOR_H */
