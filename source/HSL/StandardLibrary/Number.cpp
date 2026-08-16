#include <HSL/ScriptManager.h>
#include <HSL/StandardLibrary/Number.h>
#include <HSL/VMThread.h>

#ifdef HSL_VM
namespace HSLStdLib {
/***
 * Number.ToString
 * \desc Converts a number to a string.
 * \param n (number): Number value.
 * \paramOpt base (integer): The numerical base, or radix. (default: `10`)
 * \return string Returns a string value.
 * \ns Number
 */
VMValue Number_ToString(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckAtLeastArgCount(argCount, 1);

	int base = 10;
	if (argCount == 2) {
		base = thread->GetInteger(args, 1);
	}

	switch (args[0].Type) {
	case VAL_DECIMAL:
	case VAL_LINKED_DECIMAL: {
		float n = thread->GetDecimal(args, 0);
		char temp[16];
		snprintf(temp, sizeof temp, "%f", n);

		if (thread->Manager->Lock()) {
			VMValue string = OBJECT_VAL(thread->Manager->CopyString(temp));
			thread->Manager->Unlock();
			return string;
		}
	}
	case VAL_INTEGER:
	case VAL_LINKED_INTEGER: {
		int n = thread->GetInteger(args, 0);
		char temp[16];
		if (base == 16) {
			snprintf(temp, sizeof temp, "0x%X", n);
		}
		else {
			snprintf(temp, sizeof temp, "%d", n);
		}

		if (thread->Manager->Lock()) {
			VMValue string = OBJECT_VAL(thread->Manager->CopyString(temp));
			thread->Manager->Unlock();
			return string;
		}
	}
	default:
		thread->ThrowRuntimeError("Expected argument %d to be of type %s instead of %s.",
			1,
			"number",
			GetValueTypeString(args[0]));
	}

	return NULL_VAL;
}
/***
 * Number.AsInteger
 * \desc Converts a decimal to an integer.
 * \param n (number): Number value.
 * \return integer Returns an integer value.
 * \ns Number
 */
VMValue Number_AsInteger(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);
	return INTEGER_VAL((int)thread->GetDecimal(args, 0));
}
/***
 * Number.AsDecimal
 * \desc Converts a integer to a decimal.
 * \param n (number): Number value.
 * \return decimal Returns a decimal value.
 * \ns Number
 */
VMValue Number_AsDecimal(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);
	return DECIMAL_VAL(thread->GetDecimal(args, 0));
}
};
#endif
