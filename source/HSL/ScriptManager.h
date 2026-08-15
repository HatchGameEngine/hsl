#ifndef HSL_SCRIPTMANAGER_H
#define HSL_SCRIPTMANAGER_H

#include <HSL/Bytecode.h>
#include <HSL/GarbageCollector.h>
#include <HSL/Types.h>
#include <HSL/TypeImpl/ArrayImpl.h>
#include <HSL/TypeImpl/FunctionImpl.h>
#include <HSL/TypeImpl/InstanceImpl.h>
#include <HSL/TypeImpl/MapImpl.h>
#include <HSL/TypeImpl/StreamImpl.h>
#include <HSL/TypeImpl/StringImpl.h>
#include <HSL/VMThread.h>
#include <Exceptions/ScriptException.h>
#include <Includes/HashMap.h>
#include <Includes/Standard.h>
#include <IO/MemoryStream.h>
#include <IO/Stream.h>

#ifdef HSL_COMPILER
#include <HSL/Compiler.h>
#endif

#ifdef HSL_LIBRARY
#include <HSL/API.h>
#endif

#ifdef USE_SDL
#include <SDL2/SDL.h>
#endif

#define OBJECTS_DIR_NAME "Objects/"

#define MAX_VM_THREADS 8

class ScriptManager {
private:
#ifdef HSL_LIBRARY
	hsl_ImportScriptHandler ImportScriptHandler = nullptr;
	hsl_ImportClassHandler ImportClassHandler = nullptr;
	hsl_WithIteratorHandler WithIteratorHandler = nullptr;
#endif

#if defined(HSL_VM) && defined(VM_DEBUG)
	HashMap<SourceFile*>* SourceFiles = nullptr;

	static Uint32 GetBranchLimit();
	void InitThread(VMThread* thread);
	void LoadSourceCodeLines(SourceFile* sourceFile, char* text);
	void LoadSourceCodeLines(SourceFile* sourceFile, const char* sourceFilename);
#endif

public:
#ifdef HSL_VM
	static bool LoadAllClasses;
#ifdef VM_DEBUG
	bool BreakpointsEnabled = false;
#endif
#ifdef HSL_LIBRARY
	hsl_RuntimeErrorHandler RuntimeErrorHandler = nullptr;
	bool HasWithIteratorHandler = false;
	char* LastCompileError = nullptr;
#endif
	Obj* RootObject = nullptr;
	HashMap<VMValue>* Globals = nullptr;
	VMThread Threads[MAX_VM_THREADS];
	Uint32 ThreadCount = 0;
	GarbageCollector* GC = nullptr;
	vector<ObjModule*> ModuleList;
	vector<ObjModule*> TempModuleList;
	HashMap<BytecodeContainer>* Sources = nullptr;
	HashMap<ObjClass*>* Classes = nullptr;
	HashMap<ObjModule*>* Modules = nullptr;
	HashMap<char*>* Tokens = nullptr;
	vector<ObjNamespace*> AllNamespaces;
#endif

#ifdef USE_SDL
	SDL_mutex* GlobalLock = nullptr;
#endif

	HashMap<VMValue>* Constants = nullptr;

	ankerl::unordered_dense::map<std::string_view, ObjString*>* Strings = nullptr;

	ArrayImpl* ImplArray = nullptr;
	FunctionImpl* ImplFunction = nullptr;
	InstanceImpl* ImplInstance = nullptr;
	MapImpl* ImplMap = nullptr;
	StreamImpl* ImplStream = nullptr;
	StringImpl* ImplString = nullptr;
	HashMap<ObjClass*>* ImplClasses = nullptr;

	ScriptManager();
	~ScriptManager();

	void DestroyObject(Obj* object);
	void FreeFunction(Obj* object);
	void FreeModule(Obj* object);
	void FreeClass(Obj* object);
	void FreeEnumeration(Obj* object);
	void FreeNamespace(Obj* object);
	void FreeBoundMethod(Obj* object);
#ifdef HSL_VM
	void RemoveTemporaryModules();
	void RequestGarbageCollection(bool doLog);
	void ForceGarbageCollection(bool doLog);
	void ResetStack();
#endif
	static void Init();
	static void Dispose();
	bool Lock();
	bool Unlock();
#ifdef HSL_VM
	VMThread* NewThread();
	void DisposeThread(VMThread* thread);
	void DefineMethod(VMThread* thread, ObjFunction* function, Uint32 hash);
	void DefineNative(ObjClass* klass, const char* name, NativeFn function);
	void GlobalLinkInteger(ObjClass* klass, const char* name, int* value);
	void GlobalLinkDecimal(ObjClass* klass, const char* name, float* value);
	void GlobalConstInteger(ObjClass* klass, const char* name, int value);
	void GlobalConstDecimal(ObjClass* klass, const char* name, float value);
	ObjClass* GetClassParent(Obj* object, ObjClass* klass);
	bool GetClassMethod(ObjClass* klass, Uint32 hash, VMValue* callable);
	bool GetClassMethod(Obj* object, ObjClass* klass, Uint32 hash, VMValue* callable);
	bool ClassHasMethod(ObjClass* klass, Uint32 hash);
#endif
	void LinkStandardLibrary();
#ifdef HSL_VM
	Bytecode* ReadBytecode(BytecodeContainer bytecodeContainer);
	Bytecode* ReadBytecode(Stream* stream);
	ObjModule* LoadBytecode(Bytecode* bytecode, Uint32 filenameHash);
	ObjModule* LoadBytecode(BytecodeContainer bytecodeContainer, Uint32 filenameHash);
	ObjModule* LoadBytecode(Stream* stream, Uint32 filenameHash);
	bool RunBytecode(VMThread* thread, BytecodeContainer bytecodeContainer, Uint32 filenameHash);
	bool RunBytecode(VMThread* thread, Stream* stream, Uint32 filenameHash);
#ifdef VM_DEBUG
	void AddModuleBreakpoints(VMThread* thread, ObjModule* module);
#endif
	bool CallFunction(const char* functionName);
	VMValue FindFunction(const char* functionName);
	bool ScriptExists(const char* name);
	bool ClassExists(const char* objectName);
	bool ClassExists(Uint32 hash);
	bool IsClassLoaded(const char* className);
	bool IsStandardLibraryClass(const char* className);
	bool LoadScript(VMThread* thread, const char* filename);
#ifdef HSL_COMPILER
	bool LoadScriptFromStream(VMThread* thread, Stream* stream, const char* filename);
	ObjModule* CompileScriptFromStream(VMThread* thread, Stream* stream, const char* filename);
	ObjModule* CompileAndLoad(VMThread* thread, const char* code, const char* filename, CompilerSettings settings);
	ObjModule* CompileAndLoad(VMThread* thread, Compiler* compiler, const char* code, const char* filename);
#endif
	bool IsScriptLoaded(const char* filename);
	bool IsScriptLoaded(Uint32 filenameHash);
	ObjModule* GetScriptModule(const char* filename);
	ObjModule* GetScriptModule(Uint32 filenameHash);
	ObjFunction* GetFunctionAtScriptLine(ObjModule* module, int lineNum);
#ifdef HSL_LIBRARY
	void SetImportScriptHandler(hsl_ImportScriptHandler handler);
	void SetImportClassHandler(hsl_ImportClassHandler handler);
	void SetWithIteratorHandler(hsl_WithIteratorHandler handler);
	bool CallWithIteratorHandler(int state, VMValue receiver, int* index, VMValue* newReceiver);
#endif
	bool LoadObjectClass(VMThread* thread, const char* objectName);
	ObjClass* GetObjectClass(const char* className);
#ifdef VM_DEBUG
	char* GetSourceCodeLine(const char* sourceFilename, int line);
	void AddSourceFile(const char* sourceFilename, char* text);
	void RemoveSourceFile(const char* sourceFilename);
#endif
#endif
	static Uint32 MakeFilenameHash(const char* filename);
	static std::string GetBytecodeFilenameForHash(Uint32 filenameHash);
#ifdef HSL_VM
	int GetInteger(VMValue* args, int index, VMThread* thread);
	float GetDecimal(VMValue* args, int index, VMThread* thread);
	char* GetString(VMValue* args, int index, VMThread* thread);
	ObjString* GetVMString(VMValue* args, int index, VMThread* thread);
	ObjArray* GetArray(VMValue* args, int index, VMThread* thread);
	ObjMap* GetMap(VMValue* args, int index, VMThread* thread);
	ObjInstance* GetInstance(VMValue* args, int index, VMThread* thread);
	ObjFunction* GetFunction(VMValue* args, int index, VMThread* thread);
#endif

#ifdef HSL_VM
	static VMValue VM_GetClass(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_HasField(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_GetField(int argCount, VMValue* args, VMThread* thread);
	static VMValue VM_SetField(int argCount, VMValue* args, VMThread* thread);
#endif

	Obj* AllocateObject(size_t size, ObjType type);
	ObjString* GetInternedString(std::string_view view);
	ObjString* CreateInternedString(std::string_view view);
	ObjString* AllocateString(char* chars, size_t length);
	ObjString* TakeString(char* chars, size_t length);
	ObjString* TakeString(char* chars);
	ObjString* CopyString(const char* chars, size_t length);
	ObjString* CopyString(const char* chars);
	ObjString* CopyString(std::string path);
	ObjString* CopyString(ObjString* string);
	ObjFunction* NewFunction();
	ObjNative* NewNative(NativeFn function);
	ObjAPINative* NewAPINative(APINativeFn function);
	ObjUpvalue* NewUpvalue(VMValue* slot);
	ObjClosure* NewClosure(ObjFunction* function);
	ObjClass* NewClass(Uint32 hash);
	ObjClass* NewClass(const char* className);
	ObjInstance* NewInstance(ObjClass* klass);
	ObjBoundMethod* NewBoundMethod(ObjFunction* method, VMValue* args, Uint8 argCount);
	ObjArray* NewArray();
	ObjMap* NewMap();
	ObjNamespace* NewNamespace(Uint32 hash);
	ObjNamespace* NewNamespace(const char* nsName);
	ObjEnum* NewEnum(Uint32 hash);
	ObjModule* NewModule();
	Obj* NewNativeInstance(size_t size);

	static bool GetArity(VMValue callee, int& minArity, int& maxArity);

	std::string GetClassName(Uint32 hash);

	VMValue CastValueAsString(VMValue v);
	VMValue ConcatenateValues(VMValue va, VMValue vb);
};

#endif /* HSL_SCRIPTMANAGER_H */
