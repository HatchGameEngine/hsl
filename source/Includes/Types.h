#ifndef STANDARDLIBS_H
#define STANDARDLIBS_H

#include <cstdint>
#include <cstddef>

#if WIN32
#define snprintf _snprintf
#undef __useHeader
#undef __on_failure
#endif

typedef uint8_t Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef uint64_t Uint64;
typedef int8_t Sint8;
typedef int16_t Sint16;
typedef int32_t Sint32;
typedef int64_t Sint64;

#if defined(__unix__) && !defined(UNIX)
#define UNIX __unix__
#endif

#endif // STANDARDLIBS_H
