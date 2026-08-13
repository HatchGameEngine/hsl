#ifndef HSL_VALUES_H
#define HSL_VALUES_H

#include <HSL/Types.h>
#include <Includes/Standard.h>
#include <Utilities/PrintBuffer.h>

class Value {
public:
	static const char* GetPrintableObjectName(VMValue value);
	static const char* GetClassObjectName(ObjClass* klass);
	static const char* GetObjectTypeName(Uint32 type);
	static const char* GetObjectTypeName(VMValue value);

	static std::string ToString(VMValue v);
	static VMValue CastAsInteger(VMValue v);
	static VMValue CastAsDecimal(VMValue v);
	static bool SortaEqual(VMValue a, VMValue b);
	static bool Equal(VMValue a, VMValue b);
	static bool ExactlyEqual(VMValue a, VMValue b);
	static bool Falsey(VMValue a);
	static VMValue Delink(VMValue val);
};

#endif /* HSL_VALUES_H */
