#ifndef UTILITIES_MEMORY_H
#define UTILITIES_MEMORY_H

#include <cstdlib>

#define MEMORY_ALLOC(size) malloc((size))
#define MEMORY_CALLOC(num, size) calloc((num), (size))
#define MEMORY_REALLOC(pointer, size) realloc((pointer), (size))
#define MEMORY_FREE(pointer) free((pointer))

#endif /* UTILITIES_MEMORY_H */
