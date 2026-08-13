#ifndef STANDARDLIBS_H
#define STANDARDLIBS_H

#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <csetjmp>
#include <deque>
#include <filesystem>
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <unordered_map>
#include <vector>

#include <Includes/Endian.h>

#if WIN32
#define snprintf _snprintf
#undef __useHeader
#undef __on_failure
#endif

template<typename T>
using vector = std::vector<T>;

template<typename T1, typename T2>
using map = std::map<T1, T2>;

template<typename T>
using stack = std::stack<T>;

template<typename T>
using deque = std::deque<T>;

using string = std::string;

typedef uint8_t Uint8;
typedef uint16_t Uint16;
typedef uint32_t Uint32;
typedef uint64_t Uint64;
typedef int8_t Sint8;
typedef int16_t Sint16;
typedef int32_t Sint32;
typedef int64_t Sint64;

#define HITBOX_LEFT 0
#define HITBOX_TOP 1
#define HITBOX_RIGHT 2
#define HITBOX_BOTTOM 3
#define NUM_HITBOX_SIDES 4

#ifdef IOS
#define NEW_STRUCT_MACRO(n) (n)
#else
#define NEW_STRUCT_MACRO(n) n
#endif

#if defined(__unix__) && !defined(UNIX)
#define UNIX __unix__
#endif

#endif // STANDARDLIBS_H
