#include <Diagnostics/Memory.h>

#ifdef MEMORY_TRACKING
#include <Diagnostics/Log.h>

vector<void*> Memory::TrackedMemory;
vector<size_t> Memory::TrackedSizes;
vector<const char*> Memory::TrackedMemoryNames;
size_t Memory::MemoryUsage = 0;
bool Memory::IsTracking = false;
#endif

void* Memory::Malloc(size_t size) {
	void* mem = malloc(size);
#ifdef MEMORY_TRACKING
	if (Memory::IsTracking) {
		if (mem) {
			MemoryUsage += size;

			TrackedMemory.push_back(mem);
			TrackedSizes.push_back(size);
			TrackedMemoryNames.push_back(NULL);
		}
		else {
			Log::Print(Log::LOG_ERROR, "Could not allocate memory for Malloc!");
		}
	}
#endif
	return mem;
}
void* Memory::Calloc(size_t count, size_t size) {
	void* mem = calloc(count, size);
#ifdef MEMORY_TRACKING
	if (Memory::IsTracking) {
		if (mem) {
			MemoryUsage += count * size;

			TrackedMemory.push_back(mem);
			TrackedSizes.push_back(count * size);
			TrackedMemoryNames.push_back(NULL);
		}
		else {
			Log::Print(Log::LOG_ERROR, "Could not allocate memory for Calloc!");
		}
	}
#endif
	return mem;
}
void* Memory::Realloc(void* pointer, size_t size) {
	void* mem = realloc(pointer, size);
#ifdef MEMORY_TRACKING
	if (Memory::IsTracking) {
		if (mem) {
			for (Uint32 i = 0; i < TrackedMemory.size(); i++) {
				if (TrackedMemory[i] == pointer) {
					MemoryUsage += size - TrackedSizes[i];

					TrackedMemory[i] = mem;
					TrackedSizes[i] = size;
					return mem;
				}
			}
		}
		else {
			Log::Print(Log::LOG_ERROR, "Could not allocate memory for Realloc!");
		}
	}
#endif
	return mem;
}
// Tracking functions
void* Memory::TrackedMalloc(const char* identifier, size_t size) {
	void* mem = malloc(size);
#ifdef MEMORY_TRACKING
	if (Memory::IsTracking) {
		if (mem) {
			MemoryUsage += size;

			TrackedMemory.push_back(mem);
			TrackedSizes.push_back(size);
			TrackedMemoryNames.push_back(identifier);
		}
		else {
			Log::Print(Log::LOG_ERROR, "Could not allocate memory for TrackedMalloc!");
		}
	}
#endif
	return mem;
}
void* Memory::TrackedCalloc(const char* identifier, size_t count, size_t size) {
	void* mem = calloc(count, size);
#ifdef MEMORY_TRACKING
	if (Memory::IsTracking) {
		if (mem) {
			MemoryUsage += count * size;

			TrackedMemory.push_back(mem);
			TrackedSizes.push_back(count * size);
			TrackedMemoryNames.push_back(identifier);
		}
		else {
			Log::Print(Log::LOG_ERROR, "Could not allocate memory for TrackedCalloc!");
		}
	}
#endif
	return mem;
}
void Memory::Track(void* pointer, const char* identifier) {
#ifdef MEMORY_TRACKING
	if (Memory::IsTracking) {
		for (Uint32 i = 0; i < TrackedMemory.size(); i++) {
			if (TrackedMemory[i] == pointer) {
				TrackedMemoryNames[i] = identifier;
				return;
			}
		}
	}
#endif
}
void Memory::Track(void* pointer, size_t size, const char* identifier) {
#ifdef MEMORY_TRACKING
	if (Memory::IsTracking) {
		for (Uint32 i = 0; i < TrackedMemory.size(); i++) {
			if (TrackedMemory[i] == pointer) {
				TrackedSizes[i] = size;
				TrackedMemoryNames[i] = identifier;
				return;
			}
		}

		TrackedMemory.push_back(pointer);
		TrackedSizes.push_back(size);
		TrackedMemoryNames.push_back(identifier);
	}
#endif
}
void Memory::Free(void* pointer) {
#ifdef MEMORY_TRACKING
	if (Memory::IsTracking) {
		for (Uint32 i = 0; i < TrackedMemory.size(); i++) {
			if (TrackedMemory[i] == pointer) {
				// 32-bit
				size_t ptr_size = sizeof(void*);
				if (ptr_size == 4) {
					size_t* debug = (size_t*)TrackedMemory[i];
					for (size_t d = 0, dSz = TrackedSizes[i] / ptr_size;
						d < dSz;
						d++) {
						debug[d] = 0xCDCDCDCDU;
					}
				}
				// 64-bit
				else if (ptr_size == 8) {
					size_t* debug = (size_t*)TrackedMemory[i];
					for (size_t d = 0, dSz = TrackedSizes[i] / ptr_size;
						d < dSz;
						d++) {
						debug[d] = 0xCDCDCDCDCDCDCDCDU;
					}
				}
				break;
			}
		}
	}
	Memory::Remove(pointer);
	if (!pointer) {
		return;
	}
#endif

	free(pointer);
}
#ifdef MEMORY_TRACKING
void Memory::Remove(void* pointer) {
	if (!pointer) {
		return;
	}
	if (Memory::IsTracking) {
		for (Uint32 i = 0; i < TrackedMemory.size(); i++) {
			if (TrackedMemory[i] == pointer) {
				MemoryUsage -= TrackedSizes[i];

				TrackedMemoryNames.erase(TrackedMemoryNames.begin() + i);
				TrackedMemory.erase(TrackedMemory.begin() + i);
				TrackedSizes.erase(TrackedSizes.begin() + i);
				return;
			}
		}
	}
}
#endif

const char* Memory::GetName(void* pointer) {
#ifdef MEMORY_TRACKING
	if (Memory::IsTracking) {
		for (Uint32 i = 0; i < TrackedMemory.size(); i++) {
			if (TrackedMemory[i] == pointer) {
				return TrackedMemoryNames[i];
			}
		}
	}
#endif
	return NULL;
}

void Memory::ClearTrackedMemory() {
#ifdef MEMORY_TRACKING
	for (Uint32 i = 0; i < TrackedMemory.size(); i++) {
		Memory::Free(TrackedMemory[i]);
	}
	TrackedMemoryNames.clear();
	TrackedMemory.clear();
	TrackedSizes.clear();
#endif
}
