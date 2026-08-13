#ifndef UTILITIES_PRINTBUFFER_H
#define UTILITIES_PRINTBUFFER_H

struct PrintBuffer {
	char** Buffer;
	int WriteIndex;
	int BufferSize;
};

int buffer_printf(PrintBuffer* printBuffer, const char* format, ...);
int buffer_write(PrintBuffer* printBuffer, const char* string);
int buffer_write(PrintBuffer* printBuffer, char chr);

#endif /* UTILITIES_PRINTBUFFER_H */
