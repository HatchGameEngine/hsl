#ifndef HSL_TYPEIMPL_TYPEIMPL_H
#define HSL_TYPEIMPL_TYPEIMPL_H

class ScriptManager;

#include <HSL/Types.h>
#include <Includes/Types.h>

class TypeImpl {
private:
	static HashMap<const char*>* PrintableClassNames;

public:
	ScriptManager* Manager;
	ObjClass* Class;

	static void Init();
	static void Dispose();

	static void RegisterClass(ScriptManager* manager, ObjClass* klass);
	static void ExposeClass(ScriptManager* manager, ObjClass* klass);
};

#endif /* HSL_TYPEIMPL_TYPEIMPL_H */
