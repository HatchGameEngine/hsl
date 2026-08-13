#ifndef HSL_API_API_H
#define HSL_API_API_H

#include <HSL/API/libhsl.h>
#include <HSL/Types.h>

class Compiler;
class MemoryStream;

hsl_ValueType ValueTypeToAPIValueType(ValueType type);
hsl_ObjType ObjTypeToAPIObjType(ObjType type);

Uint32 hsl_get_hash_internal(const char* name);

hsl_Result hsl_compile_internal(Compiler* compiler, const char* code, MemoryStream** stream, const char* input_filename);

#endif /* HSL_API_API_H */
