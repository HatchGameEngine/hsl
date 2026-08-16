#ifndef HSL_STANDARDLIBRARY_FILE_H
#define HSL_STANDARDLIBRARY_FILE_H

#ifdef HSL_VM
#include <HSL/Types.h>
#include <HSL/VMThread.h>

namespace HSLStdLib {
VMValue File_Exists(int argCount, VMValue* args, VMThread* thread);
VMValue File_ReadAllText(int argCount, VMValue* args, VMThread* thread);
VMValue File_WriteAllText(int argCount, VMValue* args, VMThread* thread);
};
#endif

#endif /* HSL_STANDARDLIBRARY_FILE_H */
