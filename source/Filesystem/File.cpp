#include <Utilities/Log.h>
#include <Filesystem/File.h>
#include <IO/StandardIOStream.h>

#if WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

Stream* File::Open(const char* filename, Uint32 access) {
	Uint32 streamAccess;
	switch (access) {
	case File::READ_ACCESS:
		streamAccess = StandardIOStream::READ_ACCESS;
		break;
	case File::WRITE_ACCESS:
		streamAccess = StandardIOStream::WRITE_ACCESS;
		break;
	case File::APPEND_ACCESS:
		streamAccess = StandardIOStream::APPEND_ACCESS;
		break;
	default:
		return nullptr;
	}

	return StandardIOStream::New(filename, streamAccess);
}

bool File::Exists(const char* path) {
	Stream* stream = Open(path, READ_ACCESS);
	if (stream) {
		stream->Close();
		return true;
	}

	return false;
}
