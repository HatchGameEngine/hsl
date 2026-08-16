#ifndef HSL_VALUEPRINTER_H
#define HSL_VALUEPRINTER_H

#include <HSL/Types.h>
#include <Includes/Types.h>
#include <Utilities/PrintBuffer.h>

class ValuePrinter {
private:
	PrintBuffer* Buffer;
	bool IsJSON;
	bool PrettyPrint;

	void PrintRootValue(VMValue value);

public:
	void PrintValue(VMValue value, int indent);
	void PrintObject(VMValue value, int indent);
	void PrintArray(ObjArray* array, int indent);
	void PrintMap(ObjMap* map, int indent);

	static void Print(VMValue value);
	static void Print(PrintBuffer* buffer, VMValue value);
	static void Print(PrintBuffer* buffer, VMValue value, bool prettyPrint);
	static void Print(PrintBuffer* buffer, VMValue value, bool prettyPrint, bool isJSON);
};

#endif /* HSL_VALUEPRINTER_H */
