#ifndef IO_FILESTREAM_H
#define IO_FILESTREAM_H

#include <IO/Stream.h>
#include <Includes/Types.h>

#include <cstdio>

class FileStream : public Stream {
private:
	FILE* FilePtr;
	size_t Filesize;

public:
	static FileStream* New(const char* filename, const char* access);
	void Close();
	void Seek(Sint64 offset);
	void SeekEnd(Sint64 offset);
	void Skip(Sint64 offset);
	size_t Position();
	size_t Length();
	size_t ReadBytes(void* data, size_t n);
	size_t WriteBytes(void* data, size_t n);
};

#endif /* IO_FILESTREAM_H */
