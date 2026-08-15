#include <HSL/Bytecode.h>
#include <HSL/ScriptManager.h>
#include <HSL/TypeImpl/TypeImpl.h>
#include <HSL/Value.h>
#include <HSL/ValuePrinter.h>
#include <Diagnostics/Log.h>
#include <Filesystem/File.h>
#include <Hashing/CombinedHash.h>
#include <Utilities/StringUtils.h>

#ifdef HSL_VM
#include <HSL/GarbageCollector.h>
#endif

#ifdef HSL_COMPILER
#include <HSL/Compiler.h>
#include <Exceptions/CompilerErrorException.h>
#endif

#include <Main.h>

#ifdef HSL_VM
bool ScriptManager::LoadAllClasses = false;

// #define DEBUG_STRESS_GC

void ScriptManager::RequestGarbageCollection(bool doLog) {
	if (!GC) {
		return;
	}

#ifndef DEBUG_STRESS_GC
	if (GC->GarbageSize > GC->NextGC)
#endif
	{
		size_t startSize = GC->GarbageSize;

		ForceGarbageCollection(doLog);

		if (doLog) {
			Log::Print(Log::LOG_INFO,
				"Freed garbage from %u to %u (%d), next GC at %d",
				(Uint32)startSize,
				(Uint32)GC->GarbageSize,
				GC->GarbageSize - startSize,
				GC->NextGC);
		}
	}
}
void ScriptManager::ForceGarbageCollection(bool doLog) {
	if (Lock()) {
		if (ThreadCount > 1) {
			Unlock();
			return;
		}

		GC->Collect(doLog);

		Unlock();
	}
}

void ScriptManager::ResetStack() {
	if (ThreadCount > 0) {
		Threads[0].ResetStack();
	}
}
#endif

// #region Life Cycle
ScriptManager::ScriptManager() {
	Constants = new HashMap<VMValue>(NULL, 8);

	Strings = new ankerl::unordered_dense::map<std::string_view, ObjString*>();

#ifdef HSL_VM
	Globals = new HashMap<VMValue>(NULL, 8);
	Sources = new HashMap<BytecodeContainer>(NULL, 8);
	Classes = new HashMap<ObjClass*>(NULL, 8);
	Modules = new HashMap<ObjModule*>(NULL, 8);
	Tokens = new HashMap<char*>(NULL, 64);

#ifdef VM_DEBUG
	SourceFiles = new HashMap<SourceFile*>(NULL, 8);
#endif

	GC = new GarbageCollector(this);
#endif

	ImplClasses = new HashMap<ObjClass*>(NULL, 8);

	ImplArray = new ArrayImpl(this);
	ImplFunction = new FunctionImpl(this);
	ImplInstance = new InstanceImpl(this);
	ImplMap = new MapImpl(this);
	ImplStream = new StreamImpl(this);
	ImplString = new StringImpl(this);

#ifdef HSL_VM
	ThreadCount = 0;

	for (int i = 0; i < MAX_VM_THREADS; i++) {
		InitThread(&Threads[i]);

		Threads[i].Manager = this;
		Threads[i].ID = i;
	}

#if defined(DEVELOPER_MODE) && defined(VM_DEBUG)
#if defined(WIN32) || defined(LINUX) || defined(MACOSX)
	BreakpointsEnabled = true;
#endif
#endif
#endif

#ifdef USE_SDL
	GlobalLock = SDL_CreateMutex();
#endif
}
void ScriptManager::Init() {
	TypeImpl::Init();

#ifdef HSL_STDLIB
	StandardLibrary::Init();
#endif
}
#if defined(HSL_VM) && defined(VM_DEBUG)
Uint32 ScriptManager::GetBranchLimit() {
	int branchLimit = 0;

	return (Uint32)branchLimit;
}
#endif
ScriptManager::~ScriptManager() {
	if (Constants) {
		Constants->Clear();
		delete Constants;
		Constants = nullptr;
	}

	delete Strings;

#ifdef HSL_LIBRARY
	ImportScriptHandler = NULL;
	ImportClassHandler = NULL;
	WithIteratorHandler = NULL;
	HasWithIteratorHandler = false;

	if (LastCompileError) {
		Memory::Free(LastCompileError);
		LastCompileError = nullptr;
	}
#endif

#ifdef HSL_VM
	if (ImplClasses) {
		ImplClasses->Clear();
		delete ImplClasses;
		ImplClasses = nullptr;
	}

	if (Globals) {
		Globals->Clear();
		delete Globals;
		Globals = nullptr;
	}

	AllNamespaces.clear();
	ModuleList.clear();
	TempModuleList.clear();

	if (ThreadCount) {
		for (int i = 0; i < ThreadCount; i++) {
			if (!Threads[i].Active) {
				continue;
			}

#ifdef VM_DEBUG
			Threads[i].DisposeBreakpoints();
#endif
			Threads[i].Active = false;
		}

		Threads[0].FrameCount = 0;
		Threads[0].ResetStack();
	}

	ForceGarbageCollection(ShouldShowGarbageCollectionOutput());

	if (Sources) {
		Sources->WithAll([](Uint32 hash, BytecodeContainer bytecode) -> void {
			Memory::Free(bytecode.Data);
		});
		Sources->Clear();
		delete Sources;
		Sources = NULL;
	}
	if (Classes) {
		Classes->Clear();
		delete Classes;
		Classes = NULL;
	}
	if (Modules) {
		Modules->Clear();
		delete Modules;
		Modules = NULL;
	}
	if (Tokens) {
		Tokens->WithAll([](Uint32 hash, char* token) -> void {
			Memory::Free(token);
		});
		Tokens->Clear();
		delete Tokens;
		Tokens = NULL;
	}

	if (GC) {
		delete GC;
		GC = NULL;
	}

#ifdef VM_DEBUG
	if (SourceFiles) {
		SourceFiles->WithAll([](Uint32, SourceFile* sourceFile) -> void {
			Memory::Free(sourceFile->Text);
			delete sourceFile;
		});
		SourceFiles->Clear();
		delete SourceFiles;
		SourceFiles = NULL;
	}
#endif
#endif

	if (ImplArray) {
		delete ImplArray;
		ImplArray = nullptr;
	}
	if (ImplFunction) {
		delete ImplFunction;
		ImplFunction = nullptr;
	}
	if (ImplInstance) {
		delete ImplInstance;
		ImplInstance = nullptr;
	}
	if (ImplMap) {
		delete ImplMap;
		ImplMap = nullptr;
	}
	if (ImplStream) {
		delete ImplStream;
		ImplStream = nullptr;
	}
	if (ImplString) {
		delete ImplString;
		ImplString = nullptr;
	}

#ifdef USE_SDL
	if (GlobalLock) {
		SDL_DestroyMutex(GlobalLock);
		GlobalLock = nullptr;
	}
#endif
}
void ScriptManager::Dispose() {
	TypeImpl::Dispose();
}
void ScriptManager::FreeFunction(Obj* object) {
	ObjFunction* function = (ObjFunction*)object;

	Memory::Free(function->Name);

	function->Chunk.Free();
}
void ScriptManager::FreeModule(Obj* object) {
	ObjModule* module = (ObjModule*)object;

	for (size_t i = 0; i < module->Functions->size(); i++) {
		FreeFunction((Obj*)(*module->Functions)[i]);
	}

	Memory::Free(module->SourceFilename);

	delete module->Functions;
	delete module->Locals;
}
void ScriptManager::FreeClass(Obj* object) {
	ObjClass* klass = (ObjClass*)object;

	Memory::Free(klass->Name);

	delete klass->Methods;
	delete klass->Fields;

	if (ImplClasses) {
		ImplClasses->Remove(klass->Hash);
	}
}
void ScriptManager::FreeEnumeration(Obj* object) {
	ObjEnum* enumeration = (ObjEnum*)object;

	Memory::Free(enumeration->Name);

	delete enumeration->Fields;
}
void ScriptManager::FreeNamespace(Obj* object) {
	ObjNamespace* ns = (ObjNamespace*)object;

	Memory::Free(ns->Name);

	delete ns->Fields;
}
void ScriptManager::FreeBoundMethod(Obj* object) {
	ObjBoundMethod* boundMethod = (ObjBoundMethod*)object;

	Memory::Free(boundMethod->Arguments);
}
#ifdef HSL_VM
void ScriptManager::RemoveTemporaryModules() {
	for (size_t i = 0; i < TempModuleList.size(); i++) {
		ObjModule* module = TempModuleList[i];

#ifdef VM_DEBUG
		for (int i = 0; i < ThreadCount; i++) {
			Threads[i].RemoveBreakpointsForModule(module);
		}
#endif

		auto it = std::find(ModuleList.begin(), ModuleList.end(), module);
		if (it != ModuleList.end()) {
			ModuleList.erase(it);
		}
	}

	TempModuleList.clear();
}
void ScriptManager::InitThread(VMThread* thread) {
	memset(&thread->Stack, 0, sizeof(thread->Stack));
	memset(&thread->RegisterValue, 0, sizeof(thread->RegisterValue));
	memset(&thread->Frames, 0, sizeof(thread->Frames));
	memset(&thread->Name, 0, sizeof(thread->Name));
	memset(&thread->InstructionIgnoreMap, 0, sizeof(thread->InstructionIgnoreMap));

	thread->FrameCount = 0;
	thread->ReturnFrame = 0;

	thread->Active = false;
	thread->StackTop = thread->Stack;

#ifdef VM_DEBUG
	thread->DebugInfo = false;
	thread->AttachedDebuggerCount = 0;
	thread->CurrentBreakpointIndex = 0;
	thread->BranchLimit = GetBranchLimit();
#endif
}
VMThread* ScriptManager::NewThread() {
	for (int i = 0; i < MAX_VM_THREADS; i++) {
		VMThread* thread = &Threads[i];
		if (!thread->Active) {
			InitThread(thread);
			thread->Active = true;
			return thread;
		}
	}
	return nullptr;
}
void ScriptManager::DisposeThread(VMThread* thread) {
	thread->ResetStack();
	thread->Active = false;
}
// #endregion

// #region ValueFuncs
#endif
void ScriptManager::DestroyObject(Obj* object) {
	switch (object->Type) {
	case OBJ_STRING:
		// Remove interned string
		if (Strings) {
			ObjString* string = (ObjString*)object;
			std::string_view view(string->Chars, string->Length);
			Strings->erase(view);
		}

		ImplString->Dispose(object);
		break;
	case OBJ_ARRAY:
		ImplArray->Dispose(object);
		break;
	case OBJ_MAP:
		ImplMap->Dispose(object);
		break;
	case OBJ_MODULE:
		FreeModule(object);
		break;
	case OBJ_CLASS:
		FreeClass(object);
		break;
	case OBJ_NAMESPACE:
		FreeNamespace(object);
		break;
	case OBJ_ENUM:
		FreeEnumeration(object);
		break;
	case OBJ_BOUND_METHOD:
		FreeBoundMethod(object);
		break;
	case OBJ_INSTANCE:
	case OBJ_NATIVE_INSTANCE:
	case OBJ_ENTITY: {
		ObjInstance* instance = (ObjInstance*)object;
		if (instance->Destructor != nullptr) {
			instance->Destructor(object);
		}
		break;
	}
	default:
		break;
	}

#ifdef HSL_VM
	if (GC) {
		assert(GC->GarbageSize >= object->Size);
		GC->GarbageSize -= object->Size;
	}
#endif

	Memory::Free(object);
}
// #endregion

// #region GlobalFuncs
bool ScriptManager::Lock() {
#if defined(USE_SDL)
	return SDL_LockMutex(GlobalLock) == 0;
#else
	return LockScriptManager((void*)this);
#endif
}
bool ScriptManager::Unlock() {
#if defined(USE_SDL)
	return SDL_UnlockMutex(GlobalLock) == 0;
#else
	return UnlockScriptManager((void*)this);
#endif
}

#ifdef HSL_VM
void ScriptManager::DefineMethod(VMThread* thread, ObjFunction* function, Uint32 hash) {
	VMValue methodValue = OBJECT_VAL(function);

	ObjClass* klass = AS_CLASS(thread->Peek(0));
	klass->Methods->Put(hash, methodValue);

	if (hash == klass->Hash) {
		klass->Initializer = methodValue;
	}

	function->Class = klass;

	thread->Pop();
}
void ScriptManager::DefineNative(ObjClass* klass, const char* name, NativeFn function) {
	if (function == NULL || klass == NULL || name == NULL) {
		return;
	}

	klass->Methods->Put(name, OBJECT_VAL(NewNative(function)));
}
void ScriptManager::GlobalLinkInteger(ObjClass* klass, const char* name, int* value) {
	if (name == NULL) {
		return;
	}

	if (klass == NULL) {
		Globals->Put(name, INTEGER_LINK_VAL(value));
	}
	else {
		klass->Methods->Put(name, INTEGER_LINK_VAL(value));
	}
}
void ScriptManager::GlobalLinkDecimal(ObjClass* klass, const char* name, float* value) {
	if (name == NULL) {
		return;
	}

	if (klass == NULL) {
		Globals->Put(name, DECIMAL_LINK_VAL(value));
	}
	else {
		klass->Methods->Put(name, DECIMAL_LINK_VAL(value));
	}
}
void ScriptManager::GlobalConstInteger(ObjClass* klass, const char* name, int value) {
	if (name == NULL) {
		return;
	}
	if (klass == NULL) {
		Constants->Put(name, INTEGER_VAL(value));
	}
	else {
		klass->Methods->Put(name, INTEGER_VAL(value));
	}
}
void ScriptManager::GlobalConstDecimal(ObjClass* klass, const char* name, float value) {
	if (name == NULL) {
		return;
	}
	if (klass == NULL) {
		Constants->Put(name, DECIMAL_VAL(value));
	}
	else {
		klass->Methods->Put(name, DECIMAL_VAL(value));
	}
}
bool ScriptManager::GetClassMethod(ObjClass* klass, Uint32 hash, VMValue* callable) {
	while (klass != nullptr) {
		// Look for a field in the class which may shadow a method.
		if (klass->Fields->GetIfExists(hash, callable)) {
			return true;
		}

		// There is no field with that name, so look for methods.
		if (klass->Methods->GetIfExists(hash, callable)) {
			return true;
		}

		// Otherwise, walk up the inheritance chain until we find the method.
		klass = klass->Parent;
	}

	return false;
}
bool ScriptManager::GetClassMethod(Obj* object, ObjClass* klass, Uint32 hash, VMValue* callable) {
	while (klass != nullptr) {
		// Look for a field in the class which may shadow a method.
		if (klass->Fields->GetIfExists(hash, callable)) {
			return true;
		}

		// There is no field with that name, so look for methods.
		if (klass->Methods->GetIfExists(hash, callable)) {
			return true;
		}

		// Otherwise, walk up the inheritance chain until we find the method.
		klass = GetClassParent(object, klass);
	}

	return false;
}
ObjClass* ScriptManager::GetClassParent(Obj* object, ObjClass* klass) {
	return klass->Parent;
}
bool ScriptManager::ClassHasMethod(ObjClass* klass, Uint32 hash) {
	VMValue callable;
	return GetClassMethod(klass, hash, &callable);
}
#endif

void ScriptManager::LinkStandardLibrary() {
#ifdef HSL_STDLIB
	StandardLibrary::Link(this);
#endif
}
// #endregion

// #region ObjectFuncs
#ifdef HSL_VM
Bytecode* ScriptManager::ReadBytecode(BytecodeContainer bytecodeContainer) {
	Bytecode* bytecode = new Bytecode();
	if (!bytecode->Read(bytecodeContainer, this, Tokens)) {
		delete bytecode;
		return nullptr;
	}

	return bytecode;
}
Bytecode* ScriptManager::ReadBytecode(Stream *stream) {
	Bytecode* bytecode = new Bytecode();
	if (!bytecode->Read(stream, this, Tokens)) {
		delete bytecode;
		return nullptr;
	}

	return bytecode;
}
ObjModule* ScriptManager::LoadBytecode(Bytecode* bytecode, Uint32 filenameHash) {
	ObjModule* module = NewModule();

	for (size_t i = 0; i < bytecode->Functions.size(); i++) {
		ObjFunction* function = bytecode->Functions[i];
		Chunk* chunk = &function->Chunk;

		module->Functions->push_back(function);

		function->Module = module;
#if USING_VM_FUNCPTRS
		chunk->SetupOpfuncs();
#endif
	}

	if (bytecode->SourceFilename) {
		module->SourceFilename = StringUtils::Duplicate(bytecode->SourceFilename);
		module->HasSourceFilename = true;
	}
	else if (filenameHash) {
		char fnHash[13];
		snprintf(fnHash, sizeof(fnHash), "%08X.ibc", filenameHash);
		module->SourceFilename = StringUtils::Duplicate(fnHash);
		module->HasSourceFilename = false;
	}

	ModuleList.push_back(module);

	if (filenameHash) {
		Modules->Put(filenameHash, module);
	}
	else {
		TempModuleList.push_back(module);
	}

	return module;
}
ObjModule* ScriptManager::LoadBytecode(BytecodeContainer bytecodeContainer, Uint32 filenameHash) {
	Bytecode* bytecode = ReadBytecode(bytecodeContainer);
	if (bytecode) {
		ObjModule* module = LoadBytecode(bytecode, filenameHash);
		delete bytecode;
		return module;
	}
	return nullptr;
}
ObjModule* ScriptManager::LoadBytecode(Stream* stream, Uint32 filenameHash) {
	Bytecode* bytecode = ReadBytecode(stream);
	if (bytecode) {
		ObjModule* module = LoadBytecode(bytecode, filenameHash);
		delete bytecode;
		return module;
	}
	return nullptr;
}
bool ScriptManager::RunBytecode(VMThread* thread, BytecodeContainer bytecodeContainer, Uint32 filenameHash) {
	Bytecode* bytecode = ReadBytecode(bytecodeContainer);
	if (!bytecode) {
		return false;
	}

	ObjModule* module = LoadBytecode(bytecode, filenameHash);

	delete bytecode;

	if (!module) {
		return false;
	}

#ifdef VM_DEBUG
	if (BreakpointsEnabled) {
		AddModuleBreakpoints(thread, module);
	}
#endif

	thread->RunFunction((*module->Functions)[0], 0);

	return true;
}
bool ScriptManager::RunBytecode(VMThread* thread, Stream* stream, Uint32 filenameHash) {
	Bytecode* bytecode = ReadBytecode(stream);
	if (!bytecode) {
		return false;
	}

	ObjModule* module = LoadBytecode(bytecode, filenameHash);

	delete bytecode;

	if (!module) {
		return false;
	}

#ifdef VM_DEBUG
	if (BreakpointsEnabled) {
		AddModuleBreakpoints(thread, module);
	}
#endif

	thread->RunFunction((*module->Functions)[0], 0);

	return true;
}
#ifdef VM_DEBUG
void ScriptManager::AddModuleBreakpoints(VMThread* thread, ObjModule* module) {
	for (size_t i = 0; i < module->Functions->size(); i++) {
		thread->AddFunctionBreakpoints((*module->Functions)[i]);
	}
}
#endif
bool ScriptManager::CallFunction(const char* functionName) {
	if (!Globals->Exists(functionName)) {
		return false;
	}

	VMValue callable = Globals->Get(functionName);
	if (!IS_CALLABLE(callable)) {
		return false;
	}

	Threads[0].InvokeForEntity(callable, 0);
	return true;
}
VMValue ScriptManager::FindFunction(const char* functionName) {
	VMValue callable;

	char* methodName = StringUtils::StrCaseStr(functionName, "::");
	if (methodName) {
		std::string name = std::string(functionName, methodName - functionName);

		methodName += 2;

		if (*methodName == '\0') {
			return NULL_VAL;
		}

		if (!Globals->Exists(name.c_str())) {
			return NULL_VAL;
		}

		VMValue value = Globals->Get(name.c_str());
		if (!IS_CLASS(value)) {
			return NULL_VAL;
		}

		ObjClass* klass = AS_CLASS(value);
		Uint32 hash = Murmur::EncryptString(methodName);
		if (!GetClassMethod(klass, hash, &callable)) {
			return NULL_VAL;
		}
	}
	else {
		if (!Globals->Exists(functionName)) {
			return NULL_VAL;
		}

		callable = Globals->Get(functionName);
	}

	if (!IS_CALLABLE(callable)) {
		return NULL_VAL;
	}

	return callable;
}
bool ScriptManager::ScriptExists(const char* name) {
	return true;
}
bool ScriptManager::ClassExists(const char* objectName) {
	return true;
}
bool ScriptManager::ClassExists(Uint32 hash) {
	return true;
}
bool ScriptManager::IsClassLoaded(const char* className) {
	return ScriptManager::Classes->Exists(className);
}
bool ScriptManager::IsStandardLibraryClass(const char* className) {
	return IS_CLASS(Constants->Get(className));
}
bool ScriptManager::LoadScript(VMThread* thread, const char* filename) {
	if (!filename || !*filename) {
		return false;
	}

#ifdef HSL_LIBRARY
	if (ImportScriptHandler && ImportScriptHandler(filename, (hsl_Thread*)thread) != 0) {
		return true;
	}

	return false;
#elif defined(HSL_COMPILER)
	std::string fullPath = "";
	const char* scriptsDir = GetScriptsDirectory();
	if (scriptsDir != nullptr) {
		fullPath = std::string(scriptsDir) + "/";
	}
	fullPath += std::string(filename);
	filename = fullPath.c_str();

	Stream* stream = File::Open(filename, File::READ_ACCESS);
	if (!stream) {
		Log::Print(Log::LOG_ERROR, "Could not open file \"%s\"!", filename);
		return false;
	}

	bool succeeded = LoadScriptFromStream(thread, stream, filename);
	stream->Close();
	return succeeded;
#endif
}
#ifdef HSL_COMPILER
bool ScriptManager::LoadScriptFromStream(VMThread* thread, Stream* stream, const char* filename) {
	ObjModule* module = CompileScriptFromStream(thread, stream, filename);
	if (!module) {
		return false;
	}

	ObjFunction* function = (*module->Functions)[0];
	if (thread->Call(function, 0)) {
		thread->RunInstructionSet();
		return true;
	}

	return false;
}
ObjModule* ScriptManager::CompileScriptFromStream(VMThread* thread, Stream* stream, const char* filename) {
	size_t size = stream->Length();
	char* code = (char*)Memory::Calloc(size + 1, sizeof(char));
	if (!code) {
		Log::Print(Log::LOG_ERROR, "Out of memory reading script \"%s\"!", filename);
		return nullptr;
	}
	stream->ReadBytes(code, size);

	ObjModule* module = nullptr;
	try {
		module = CompileAndLoad(thread, code, filename, Compiler::Settings);
	} catch (const CompilerErrorException& error) {
		Log::Print(Log::LOG_ERROR, "Could not compile script \"%s\"!\n%s", filename, error.what());
	}
	Memory::Free(code);
	return module;
}
ObjModule* ScriptManager::CompileAndLoad(VMThread* thread, const char* code, const char* filename, CompilerSettings settings) {
	Compiler::PrepareCompiling();

	Compiler* compiler = new Compiler(this);
	compiler->InREPL = false;
	compiler->CurrentSettings = settings;
	compiler->Type = FUNCTIONTYPE_TOPLEVEL;
	compiler->ScopeDepth = 0;
	compiler->Initialize();
	compiler->SetupLocals();

	ObjModule* module = nullptr;
	try {
		module = CompileAndLoad(thread, compiler, code, filename);
	} catch (const CompilerErrorException& error) {
		delete compiler;
		Compiler::FinishCompiling();
		throw error;
	}

	delete compiler;
	Compiler::FinishCompiling();

	return module;
}
ObjModule* ScriptManager::CompileAndLoad(VMThread* thread, Compiler* compiler, const char* code, const char* filename) {
	bool didCompile = false;

	MemoryStream* memStream = MemoryStream::New(0x100);
	if (!memStream) {
		return nullptr;
	}

	try {
		didCompile = compiler->Compile(filename, code, memStream);
	} catch (const CompilerErrorException& error) {
		memStream->Close();

		throw error;
	}

	ObjModule* module = nullptr;

	if (didCompile) {
		Uint32 filenameHash = 0x00000000;
		if (filename) {
			filenameHash = MakeFilenameHash(filename);
		}

		memStream->Seek(0);

		module = LoadBytecode(memStream, filenameHash);

#ifdef VM_DEBUG
		if (module && BreakpointsEnabled) {
			AddModuleBreakpoints(thread, module);
		}
#endif
	}

	memStream->Close();

	return module;
}
#endif
bool ScriptManager::IsScriptLoaded(const char* filename) {
	Uint32 hash = MakeFilenameHash(filename);
	return IsScriptLoaded(hash);
}
bool ScriptManager::IsScriptLoaded(Uint32 filenameHash) {
	return Sources->Exists(filenameHash);
}
ObjModule* ScriptManager::GetScriptModule(const char* filename) {
	Uint32 hash = MakeFilenameHash(filename);
	return GetScriptModule(hash);
}
ObjModule* ScriptManager::GetScriptModule(Uint32 filenameHash) {
	if (Modules->Exists(filenameHash)) {
		return Modules->Get(filenameHash);
	}
	return nullptr;
}
ObjFunction* ScriptManager::GetFunctionAtScriptLine(ObjModule* module, int lineNum) {
	if (lineNum == 0) {
		return (*module->Functions)[0];
	}

	for (size_t i = 0; i < module->Functions->size(); i++) {
		ObjFunction* function = (*module->Functions)[i];
		Chunk* chunk = &function->Chunk;
		if (!chunk->Lines) {
			continue;
		}

		for (int offset = 0; offset < chunk->Count;) {
			int line = chunk->Lines[offset] & 0xFFFF;
			if (line == lineNum) {
				return function;
			}

			offset += Bytecode::GetTotalOpcodeSize(chunk->Code + offset);
		}
	}

	return nullptr;
}
#ifdef HSL_LIBRARY
void ScriptManager::SetImportScriptHandler(hsl_ImportScriptHandler handler) {
	ImportScriptHandler = handler;
}
void ScriptManager::SetImportClassHandler(hsl_ImportClassHandler handler) {
	ImportClassHandler = handler;
}
void ScriptManager::SetWithIteratorHandler(hsl_WithIteratorHandler handler) {
	WithIteratorHandler = handler;
	HasWithIteratorHandler = handler != nullptr;
}
bool ScriptManager::CallWithIteratorHandler(int state, VMValue receiver, int* index, VMValue* newReceiver) {
	int result = WithIteratorHandler(state, (hsl_Value*)&receiver, index, (hsl_Value*)newReceiver);

	return result != 0;
}
#endif
bool ScriptManager::LoadObjectClass(VMThread* thread, const char* objectName) {
	bool succeeded = true;

	if (ScriptManager::IsClassLoaded(objectName)) {
		return true;
	}

	if (!ScriptManager::ClassExists(objectName)) {
		return false;
	}

#ifdef HSL_LIBRARY
	if (!ImportClassHandler) {
		return false;
	}

	if (ImportClassHandler(objectName, (hsl_Thread*)thread) != 0) {
		succeeded = true;
	}
#endif

	if (succeeded && !IsStandardLibraryClass(objectName) && !Classes->Exists(objectName)) {
		ObjClass* klass = GetObjectClass(objectName);
		if (!klass) {
			Log::Print(Log::LOG_ERROR, "Could not find class of %s!", objectName);
			return false;
		}
		Classes->Put(objectName, klass);
	}

	return succeeded;
}
ObjClass* ScriptManager::GetObjectClass(const char* className) {
	VMValue value = Globals->Get(className);

	if (IS_CLASS(value)) {
		return AS_CLASS(value);
	}

	return nullptr;
}
#ifdef VM_DEBUG
void ScriptManager::LoadSourceCodeLines(SourceFile* sourceFile, char* text) {
	char* ptr = text;
	char* end = text + strlen(text);
	char* lineStart = ptr;

	while (true) {
		if (*ptr == '\n' || ptr == end) {
			sourceFile->Lines.push_back(lineStart);

			if (ptr == end) {
				break;
			}
			else {
				*ptr = '\0';
				lineStart = ptr + 1;
			}
		}

		ptr++;
	}

	sourceFile->Text = text;
	sourceFile->Exists = true;
}
void ScriptManager::LoadSourceCodeLines(SourceFile* sourceFile, const char* sourceFilename) {
	Stream* stream = File::Open(sourceFilename, File::READ_ACCESS);
	if (!stream) {
		return;
	}

	size_t size = stream->Length();
	char* text = (char*)Memory::Calloc(size + 1, sizeof(char));
	stream->ReadBytes(text, size);
	stream->Close();

	LoadSourceCodeLines(sourceFile, text);
}
char* ScriptManager::GetSourceCodeLine(const char* sourceFilename, int line) {
	SourceFile* sourceFile = nullptr;

	if (!SourceFiles->Exists(sourceFilename)) {
		sourceFile = new SourceFile;

		if (File::Exists(sourceFilename)) {
			LoadSourceCodeLines(sourceFile, sourceFilename);
		}

		if (!sourceFile->Exists) {
			Log::Print(Log::LOG_WARN, "Source file \"%s\" does not exist.", sourceFilename);
		}

		SourceFiles->Put(sourceFilename, sourceFile);
	}
	else {
		sourceFile = SourceFiles->Get(sourceFilename);
	}

	if (!sourceFile->Exists || line < 1 || line > sourceFile->Lines.size()) {
		return nullptr;
	}

	return sourceFile->Lines[line - 1];
}
void ScriptManager::AddSourceFile(const char* sourceFilename, char* text) {
	SourceFile* sourceFile = new SourceFile;
	sourceFile->Exists = true;

	LoadSourceCodeLines(sourceFile, text);

	RemoveSourceFile(sourceFilename);
	SourceFiles->Put(sourceFilename, sourceFile);
}
void ScriptManager::RemoveSourceFile(const char* sourceFilename) {
	if (SourceFiles->Exists(sourceFilename)) {
		Memory::Free(SourceFiles->Get(sourceFilename));

		SourceFiles->Remove(sourceFilename);
	}
}
#endif
#endif
Uint32 ScriptManager::MakeFilenameHash(const char* filename) {
	size_t length = strlen(filename);
	const char* dot = strrchr(filename, '.');
	if (dot) {
		length = dot - filename;
	}
	return CombinedHash::EncryptData((const void*)filename, length);
}
std::string ScriptManager::GetBytecodeFilenameForHash(Uint32 filenameHash) {
	char filename[sizeof(OBJECTS_DIR_NAME) + 12];
	snprintf(filename, sizeof filename, "%s%08X.ibc", OBJECTS_DIR_NAME, filenameHash);
	return std::string(filename);
}
// #endregion

#define ALLOCATE_OBJ(type, objectType) (type*)AllocateObject(sizeof(type), objectType)
#define ALLOCATE(type, size) (type*)Memory::TrackedMalloc(#type, sizeof(type) * size)

Obj* ScriptManager::AllocateObject(size_t size, ObjType type) {
#ifdef HSL_VM
	if (GC) {
		// Only do this when allocating more memory
		GC->GarbageSize += size;
	}
#endif

	Obj* object = (Obj*)Memory::TrackedCalloc("AllocateObject", 1, size);
	object->Size = size;
	object->Type = type;
#ifdef HSL_VM
	object->Next = RootObject;
	RootObject = object;
#endif

	return object;
}

ObjString* ScriptManager::GetInternedString(std::string_view view) {
	if (Strings->count(view) > 0) {
		return (*Strings)[view];
	}

	return nullptr;
}

ObjString* ScriptManager::CreateInternedString(std::string_view view) {
	ObjString* string = (ObjString*)ImplString->New((char*)view.data(), view.length());

	(*Strings)[view] = string;

	return string;
}

ObjString* ScriptManager::AllocateString(char* chars, size_t length) {
	std::string_view view(chars, length);

	ObjString* string = GetInternedString(view);
	if (string) {
		return string;
	}

	return CreateInternedString(view);
}

ObjString* ScriptManager::TakeString(char* chars, size_t length) {
	std::string_view view(chars, length);

	ObjString* string = GetInternedString(view);
	if (string) {
		// This string was already interned, so we have to free chars
		Memory::Free(chars);
		return string;
	}

	return CreateInternedString(view);
}
ObjString* ScriptManager::TakeString(char* chars) {
	return TakeString(chars, strlen(chars));
}
ObjString* ScriptManager::CopyString(const char* chars, size_t length) {
	char* heapChars = ALLOCATE(char, length + 1);
	if (!heapChars) {
		return nullptr;
	}

	if (chars) {
		memcpy(heapChars, chars, length);
	}

	heapChars[length] = '\0';

	return AllocateString(heapChars, length);
}
ObjString* ScriptManager::CopyString(const char* chars) {
	return CopyString(chars, strlen(chars));
}
ObjString* ScriptManager::CopyString(std::string string) {
	return CopyString(string.c_str());
}
ObjString* ScriptManager::CopyString(ObjString* string) {
	char* heapChars = ALLOCATE(char, string->Length + 1);
	if (!heapChars) {
		return nullptr;
	}

	memcpy(heapChars, string->Chars, string->Length);
	heapChars[string->Length] = '\0';

	return AllocateString(heapChars, string->Length);
}

#ifdef HSL_VM
VMValue ScriptManager::VM_GetClass(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);

	if (IS_OBJECT(args[0])) {
		return OBJECT_VAL(AS_OBJECT(args[0])->Class);
	}

	return NULL_VAL;
}

VMValue ScriptManager::VM_HasField(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);

	const char* name = thread->GetString(args, 1);
	Uint32 hash = Murmur::EncryptString(name);

	return INTEGER_VAL(thread->HasProperty(args[0], hash));
}

VMValue ScriptManager::VM_GetField(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);

	const char* name = thread->GetString(args, 1);
	Uint32 hash = Murmur::EncryptString(name);

	if (thread->HasProperty(args[0], hash))
		return thread->GetProperty(args[0], hash);

	thread->ThrowRuntimeError(false, "Could not find %s in %s!", name, GetValueTypeString(args[0]));
	return NULL_VAL;
}

VMValue ScriptManager::VM_SetField(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 3);

	const char* name = thread->GetString(args, 1);
	Uint32 hash = Murmur::EncryptString(name);

	return thread->SetProperty(args[0], hash, args[2]);;
}
#endif

ObjFunction* ScriptManager::NewFunction() {
	return (ObjFunction*)ImplFunction->New();
}
ObjNative* ScriptManager::NewNative(NativeFn function) {
	ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE_FUNCTION);
	Memory::Track(native, "NewNative");
	native->Function = function;
	return native;
}
ObjAPINative* ScriptManager::NewAPINative(APINativeFn function) {
	ObjAPINative* native = ALLOCATE_OBJ(ObjAPINative, OBJ_API_NATIVE_FUNCTION);
	Memory::Track(native, "NewAPINative");
	native->Function = function;
	return native;
}
ObjUpvalue* ScriptManager::NewUpvalue(VMValue* slot) {
	ObjUpvalue* upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
	upvalue->Closed = NULL_VAL;
	upvalue->Value = slot;
	return upvalue;
}
ObjClosure* ScriptManager::NewClosure(ObjFunction* function) {
#if 0
	ObjUpvalue** upvalues = ALLOCATE(ObjUpvalue*, function->UpvalueCount);
	for (int i = 0; i < function->UpvalueCount; i++) {
		upvalues[i] = NULL;
	}

	ObjClosure* closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
	closure->Function = function;
	closure->Upvalues = upvalues;
	closure->UpvalueCount = function->UpvalueCount;
	return closure;
#else
	return nullptr;
#endif
}
ObjClass* ScriptManager::NewClass(Uint32 hash) {
	ObjClass* klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
	Memory::Track(klass, "NewClass");
	klass->Hash = hash;
	klass->Methods = new Table(NULL, 4);
	klass->Fields = new Table(NULL, 16);
	klass->Initializer = NULL_VAL;
	klass->Name = StringUtils::Create(GetClassName(hash));
#ifdef HSL_VM
	DefineNative(klass, "GetClass", VM_GetClass);
	DefineNative(klass, "HasField", VM_HasField);
	DefineNative(klass, "GetField", VM_GetField);
	DefineNative(klass, "SetField", VM_SetField);
#endif
	return klass;
}
ObjClass* ScriptManager::NewClass(const char* className) {
	ObjClass* klass = NewClass(GetClassHash(className));
	klass->Name = (char*)Memory::Realloc(klass->Name, strlen(className) + 1);
	memcpy(klass->Name, className, strlen(className) + 1);
	return klass;
}
ObjInstance* ScriptManager::NewInstance(ObjClass* klass) {
	ObjInstance* instance = (ObjInstance*)ImplInstance->New(sizeof(ObjInstance), OBJ_INSTANCE);
	instance->Object.Class = klass;
	return instance;
}
ObjBoundMethod* ScriptManager::NewBoundMethod(ObjFunction* method, VMValue* args, Uint8 argCount) {
	ObjBoundMethod* bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
	Memory::Track(bound, "NewBoundMethod");
	bound->Method = method;
	bound->Arguments = (VMValue*)Memory::Malloc(argCount * sizeof(VMValue));
	bound->ArgumentCount = argCount;
	memcpy(bound->Arguments, args, argCount * sizeof(VMValue));
	return bound;
}
ObjArray* ScriptManager::NewArray() {
	return (ObjArray*)ImplArray->Allocate();
}
ObjMap* ScriptManager::NewMap() {
	return (ObjMap*)ImplMap->Allocate();
}
ObjNamespace* ScriptManager::NewNamespace(Uint32 hash) {
	ObjNamespace* ns = ALLOCATE_OBJ(ObjNamespace, OBJ_NAMESPACE);
	Memory::Track(ns, "NewNamespace");
	ns->Fields = new Table(NULL, 16);
	ns->Name = StringUtils::Create(GetClassName(hash));
	return ns;
}
ObjNamespace* ScriptManager::NewNamespace(const char* nsName) {
	ObjNamespace* ns = NewNamespace(GetClassHash(nsName));
	ns->Name = (char*)Memory::Realloc(ns->Name, strlen(nsName) + 1);
	memcpy(ns->Name, nsName, strlen(nsName) + 1);
	return ns;
}
ObjEnum* ScriptManager::NewEnum(Uint32 hash) {
	ObjEnum* enumeration = ALLOCATE_OBJ(ObjEnum, OBJ_ENUM);
	Memory::Track(enumeration, "NewEnum");
	enumeration->Fields = new Table(NULL, 16);
	enumeration->Name = StringUtils::Create(GetClassName(hash));
	return enumeration;
}
ObjModule* ScriptManager::NewModule() {
	ObjModule* module = ALLOCATE_OBJ(ObjModule, OBJ_MODULE);
	Memory::Track(module, "NewModule");
	module->Functions = new vector<ObjFunction*>();
	module->Locals = new vector<VMValue>();
	return module;
}
Obj* ScriptManager::NewNativeInstance(size_t size) {
	Obj* obj = ImplInstance->New(size, OBJ_NATIVE_INSTANCE);
	Memory::Track(obj, "NewNativeInstance");
	return obj;
}

bool ScriptManager::GetArity(VMValue callee, int& minArity, int& maxArity) {
	if (IS_OBJECT(callee)) {
		switch (OBJECT_TYPE(callee)) {
		case OBJ_BOUND_METHOD: {
			ObjBoundMethod* bound = AS_BOUND_METHOD(callee);
			minArity = bound->Method->MinArity;
			maxArity = bound->Method->Arity;
			return true;
		}
		case OBJ_FUNCTION: {
			ObjFunction* function = AS_FUNCTION(callee);
			minArity = function->MinArity;
			maxArity = function->Arity;
			return true;
		}
		// No way to know. (Yet)
		case OBJ_NATIVE_FUNCTION:
		case OBJ_API_NATIVE_FUNCTION:
		default:
			break;
		}
	}
	return false;
}

std::string ScriptManager::GetClassName(Uint32 hash) {
#ifdef HSL_VM
	if (Tokens && Tokens->Exists(hash)) {
		char* t = Tokens->Get(hash);
		return std::string(t);
	}
	else
#endif
	{
		char nameHash[9];
		snprintf(nameHash, sizeof(nameHash), "%8X", hash);
		return std::string(nameHash);
	}
}

VMValue ScriptManager::CastValueAsString(VMValue v) {
	if (IS_STRING(v)) {
		return v;
	}

	char* buffer = (char*)malloc(512);
	PrintBuffer buffer_info;
	buffer_info.Buffer = &buffer;
	buffer_info.WriteIndex = 0;
	buffer_info.BufferSize = 512;
	ValuePrinter::Print(&buffer_info, v, false);
	v = OBJECT_VAL(CopyString(buffer, buffer_info.WriteIndex));
	free(buffer);
	return v;
}
VMValue ScriptManager::ConcatenateValues(VMValue va, VMValue vb) {
	ObjString* a = AS_STRING(va);
	ObjString* b = AS_STRING(vb);

	size_t length = a->Length + b->Length;
	char* chars = (char*)Memory::Malloc(length + 1);
	if (!chars) {
		return NULL_VAL;
	}

	memcpy(chars, a->Chars, a->Length);
	memcpy(chars + a->Length, b->Chars, b->Length);
	chars[length] = 0;

	ObjString* result = TakeString(chars, length);
	return OBJECT_VAL(result);
}
