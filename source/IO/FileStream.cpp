#if WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif

#include <IO/FileStream.h>

FileStream* FileStream::New(const char* filename, const char* access) {
	// Cannot open file stream if there is a directory with that name.
#if WIN32
	DWORD ftyp = GetFileAttributesA(filename);
	if (ftyp == INVALID_FILE_ATTRIBUTES || ftyp & FILE_ATTRIBUTE_DIRECTORY) {
		return nullptr;
	}
#else
	struct stat pathStat;
	if (stat(filename, &pathStat) == 0 && S_ISDIR(pathStat.st_mode)) {
		return nullptr;
	}
#endif

	FILE* file = fopen(filename, access);
	if (!file) {
		return nullptr;
	}

	FileStream* stream = new (std::nothrow) FileStream;
	if (!stream) {
		return nullptr;
	}

	stream->FilePtr = file;

	fseek(file, 0, SEEK_END);
	stream->Filesize = ftell(file);
	fseek(file, 0, SEEK_SET);

	return stream;
}

void FileStream::Close() {
	fclose(FilePtr);
	FilePtr = nullptr;
	Stream::Close();
}
void FileStream::Seek(Sint64 offset) {
	fseek(FilePtr, offset, SEEK_SET);
}
void FileStream::SeekEnd(Sint64 offset) {
	fseek(FilePtr, offset, SEEK_END);
}
void FileStream::Skip(Sint64 offset) {
	fseek(FilePtr, offset, SEEK_CUR);
}
size_t FileStream::Position() {
	return ftell(FilePtr);
}
size_t FileStream::Length() {
	return Filesize;
}

size_t FileStream::ReadBytes(void* data, size_t n) {
	return fread(data, 1, n, FilePtr);
}

size_t FileStream::WriteBytes(void* data, size_t n) {
	return fwrite(data, 1, n, FilePtr);
}
