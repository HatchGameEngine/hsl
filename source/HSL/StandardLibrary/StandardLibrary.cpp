#include <HSL/StandardLibrary/File.h>
#include <HSL/StandardLibrary/Number.h>
#include <HSL/StandardLibrary/StandardLibrary.h>
#include <HSL/Types.h>

void StandardLibrary::Link(ScriptManager* manager) {
	ObjClass* klass = nullptr;

#define INIT_CLASS(className) \
	klass = manager->NewClass(#className); \
	manager->Constants->Put(#className, OBJECT_VAL(klass))

#define DEF_NATIVE(className, funcName) \
	manager->DefineNative(klass, #funcName, HSLStdLib::className##_##funcName) \

#ifdef HSL_VM
	// #region File
	/***
	* \class File
	* \desc Filesystem file manipulation.
	*/
	INIT_CLASS(File);
	DEF_NATIVE(File, Exists);
	DEF_NATIVE(File, ReadAllText);
	DEF_NATIVE(File, WriteAllText);
	// #endregion

	// #region Number
	/***
	* \class Number
	* \desc Number type related functions.
	*/
	INIT_CLASS(Number);
	DEF_NATIVE(Number, ToString);
	DEF_NATIVE(Number, AsInteger);
	DEF_NATIVE(Number, AsDecimal);
	// #endregion
#endif

#undef DEF_NATIVE
#undef INIT_CLASS
}
