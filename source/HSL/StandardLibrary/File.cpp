#include <HSL/ScriptManager.h>
#include <HSL/StandardLibrary/File.h>
#include <HSL/VMThread.h>
#include <IO/FileStream.h>
#include <Utilities/Memory.h>

#ifdef HSL_VM
namespace HSLStdLib {
/***
 * File.Exists
 * \desc Determines if the file at the path exists.
 * \param path (string): The path of the file to check for existence.
 * \return boolean Returns whether the file exists.
 * \ns File
 */
VMValue File_Exists(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);
	char* filePath = thread->GetString(args, 0);

	bool exists = false;
	FILE* file = fopen(filePath, "rb");
	if (file) {
		exists = true;
		fclose(file);
	}

	return INTEGER_VAL(exists);
}
/***
 * File.ReadAllText
 * \desc Reads all text from the given filename.
 * \param path (string): The path of the file to read.
 * \return string Returns all the text in the file as a string value if it can be read, otherwise it returns a `null` value if it cannot be read.
 * \ns File
 */
VMValue File_ReadAllText(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 1);
	char* filePath = thread->GetString(args, 0);

	Stream* stream = FileStream::New(filePath, "rb");
	if (!stream) {
		return NULL_VAL;
	}

	if (thread->Manager->Lock()) {
		size_t size = stream->Length();
		char* text = (char*)MEMORY_ALLOC(size + 1);
		if (!text) {
			thread->Manager->Unlock();
			return NULL_VAL;
		}
		stream->ReadBytes(text, size);
		text[size] = '\0'; // Ensure it's NULL-terminated

		ObjString* string = thread->Manager->TakeString(text, size);
		thread->Manager->Unlock();
		return OBJECT_VAL(string);
	}

	return NULL_VAL;
}
/***
 * File.WriteAllText
 * \desc Writes all text to the given filename.
 * \param path (string): The path of the file to read.
 * \param text (string): The text to write to the file.
 * \return boolean Returns whether the file was written successfully.
 * \ns File
 */
VMValue File_WriteAllText(int argCount, VMValue* args, VMThread* thread) {
	thread->CheckArgCount(argCount, 2);

	char* filePath = thread->GetString(args, 0);
	ObjString* text = AS_STRING(args[1]);

	if (thread->Manager->Lock()) {
		Stream* stream = FileStream::New(filePath, "wb");
		if (!stream) {
			thread->Manager->Unlock();
			return INTEGER_VAL(false);
		}

		stream->WriteBytes(text->Chars, text->Length);
		stream->Close();

		thread->Manager->Unlock();
		return INTEGER_VAL(true);
	}

	return INTEGER_VAL(false);
}
};
#endif
