#ifndef HSL_STANDARDLIBRARY_NUMBER_H
#define HSL_STANDARDLIBRARY_NUMBER_H

#ifdef HSL_VM
#include <HSL/Types.h>
#include <HSL/VMThread.h>

namespace HSLStdLib {
VMValue Number_ToString(int argCount, VMValue* args, VMThread* thread);
VMValue Number_AsInteger(int argCount, VMValue* args, VMThread* thread);
VMValue Number_AsDecimal(int argCount, VMValue* args, VMThread* thread);
};
#endif

#endif /* HSL_STANDARDLIBRARY_NUMBER_H */
