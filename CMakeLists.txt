cmake_minimum_required(VERSION 3.14)

# set the project name and version
project(hsl VERSION 0.0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Default to Debug build if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

# Get Git commit hash
find_file(REPO_DIR NAMES .git PATHS ${CMAKE_SOURCE_DIR} NO_DEFAULT_PATH)

if (REPO_DIR)
  execute_process(
    COMMAND git log -1 --format=%H
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

# Build options
option(USING_HSL_VM "Build with HSL VM" ON)
option(USING_HSL_COMPILER "Build with HSL compiler" ON)
option(USING_HSL_STANDARD_LIBRARY "Build with HSL standard library" ON)
option(ENABLE_USING_VM_FUNCPTRS "Enable function-pointer based VM" ON)

if(${CMAKE_BUILD_TYPE} MATCHES "Debug")
  option(ENABLE_VM_DEBUGGING "Enable VM debugging" ON)
  option(DEVELOPER_MODE "Developer mode" ON)
else()
  option(ENABLE_VM_DEBUGGING "Enable VM debugging" OFF)
  option(DEVELOPER_MODE "Developer mode" OFF)
endif()

option(BUILD_HSL_PROGRAM "Build hsl program" ON)
option(BUILD_HSLC_PROGRAM "Build hslc program" ON)
option(BUILD_LIBHSL "Build libhsl" ON)

option(ENABLE_USING_LINENOISE "Use linenoise-ng for the CLI" ON)
option(ENABLE_HAVE_NANOPRINTF "Build with nanoprintf" ON)

# HSL library
if(BUILD_LIBHSL)
  add_library(libhsl SHARED
    ${HSL_MAIN_SOURCES}
    source/HSL/API/API.cpp
    source/HSL/API/Array.cpp
    source/HSL/API/Call.cpp
    source/HSL/API/Callable.cpp
    source/HSL/API/Class.cpp
    source/HSL/API/Compiler.cpp
    source/HSL/API/Context.cpp
    source/HSL/API/Global.cpp
    source/HSL/API/Instance.cpp
    source/HSL/API/Module.cpp
    source/HSL/API/Native.cpp
    source/HSL/API/Object.cpp
    source/HSL/API/Script.cpp
    source/HSL/API/Stack.cpp
    source/HSL/API/Thread.cpp
    source/HSL/API/Value.cpp
    source/Main.cpp
  )
  set_target_properties(libhsl PROPERTIES OUTPUT_NAME hsl)
  set_target_properties(libhsl PROPERTIES VERSION 0.0.1 SOVERSION 0)

  target_compile_definitions(libhsl PRIVATE -DHSL_LIBRARY -DNO_SCRIPT_COMPILING -DTARGET_NAME=\"libhsl\")
  if(DEFINED GIT_COMMIT_HASH)
    target_compile_definitions(libhsl PRIVATE "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
  endif()
  target_include_directories(libhsl PUBLIC
    source
    third_party
  )
  target_compile_definitions(libhsl PRIVATE)
  if(USING_HSL_VM)
    target_compile_definitions(libhsl PRIVATE -DHSL_VM -DHSL_STANDALONE_RUNNER)
    target_sources(libhsl PUBLIC ${HSL_VM_SOURCES})
  endif()
  if(USING_HSL_COMPILER)
    target_compile_definitions(libhsl PRIVATE -DHSL_COMPILER)
    target_sources(libhsl PUBLIC ${HSL_COMPILER_SOURCES})
  endif()
  if(USING_HSL_VM AND ENABLE_VM_DEBUGGING)
    target_compile_definitions(libhsl PRIVATE -DVM_DEBUG)
  endif()
  if(USING_HSL_VM AND ENABLE_USING_VM_FUNCPTRS)
    target_compile_definitions(libhsl PRIVATE -DUSING_VM_FUNCPTRS)
  endif()
  if(ENABLE_HAVE_NANOPRINTF)
    target_compile_definitions(libhsl PRIVATE -DHAVE_NANOPRINTF)
  endif()
endif()

set(HSL_MAIN_SOURCES
  source/HSL/Bytecode.cpp
  source/HSL/BytecodeDebugger.cpp
  source/HSL/ScriptManager.cpp
  source/HSL/ScriptREPL.cpp
  source/HSL/TypeImpl/ArrayImpl.cpp
  source/HSL/TypeImpl/FunctionImpl.cpp
  source/HSL/TypeImpl/InstanceImpl.cpp
  source/HSL/TypeImpl/MapImpl.cpp
  source/HSL/TypeImpl/StreamImpl.cpp
  source/HSL/TypeImpl/StringImpl.cpp
  source/HSL/TypeImpl/TypeImpl.cpp
  source/HSL/Types.cpp
  source/HSL/Value.cpp
  source/HSL/ValuePrinter.cpp
  source/Hashing/CombinedHash.cpp
  source/Hashing/CRC32.cpp
  source/Hashing/FNV1A.cpp
  source/Hashing/MD5.cpp
  source/Hashing/Murmur.cpp
  source/IO/FileStream.cpp
  source/IO/MemoryStream.cpp
  source/IO/Stream.cpp
  source/IO/TerminalInput.cpp
  source/Utilities/Log.cpp
  source/Utilities/PrintBuffer.cpp
  source/Utilities/StringUtils.cpp
)
set(HSL_VM_SOURCES
  source/HSL/GarbageCollector.cpp
  source/HSL/VMThread.cpp
  source/HSL/VMThreadDebugger.cpp
)
set(HSL_COMPILER_SOURCES
  source/HSL/Compiler.cpp
)

# HSL standalone program
if(BUILD_HSL_PROGRAM)
  add_executable(hsl ${HSL_MAIN_SOURCES} source/Main.cpp source/EntryPoint.cpp)

  option(ENABLE_REPL "Enable REPL" ON)
  option(ENABLE_LOG_FILE "Enable logging output to a file" ON)
  if(USING_HSL_STANDARD_LIBRARY)
    option(USING_CLOCK "Build with clock" ON)
  else()
    option(USING_CLOCK "Build with clock" OFF)
  endif()

  target_compile_definitions(hsl PRIVATE -DNO_SCRIPT_COMPILING -DTARGET_NAME=\"hsl\")
  if(DEFINED GIT_COMMIT_HASH)
    target_compile_definitions(hsl PRIVATE "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
  endif()
  target_include_directories(hsl PUBLIC
    source
    third_party
  )
  if(ENABLE_LOG_FILE)
    target_compile_definitions(hsl PRIVATE -DUSE_LOG_FILE)
  endif()
  if(USING_CLOCK)
    target_compile_definitions(hsl PRIVATE -DUSE_CLOCK)
    target_sources(hsl PUBLIC source/Utilities/Clock.cpp)
  endif()
  if(USING_HSL_VM)
    target_compile_definitions(hsl PRIVATE -DHSL_VM -DHSL_STANDALONE_RUNNER)
    target_sources(hsl PUBLIC ${HSL_VM_SOURCES})
  endif()
  if(USING_HSL_COMPILER)
    target_compile_definitions(hsl PRIVATE -DHSL_COMPILER)
    target_sources(hsl PUBLIC ${HSL_COMPILER_SOURCES})
  endif()
  if(USING_HSL_VM AND ENABLE_VM_DEBUGGING)
    target_compile_definitions(hsl PRIVATE -DVM_DEBUG)
  endif()
  if(USING_HSL_VM AND ENABLE_USING_VM_FUNCPTRS)
    target_compile_definitions(hsl PRIVATE -DUSING_VM_FUNCPTRS)
  endif()
  if(ENABLE_USING_LINENOISE)
    target_sources(hsl PUBLIC
      third_party/linenoise-ng/linenoise.cpp
      third_party/linenoise-ng/wcwidth.cpp
    )
    target_compile_definitions(hsl PRIVATE -DUSING_LINENOISE)
  endif()
  if(ENABLE_HAVE_NANOPRINTF)
    target_compile_definitions(hsl PRIVATE -DHAVE_NANOPRINTF)
  endif()
  if(ENABLE_REPL)
    target_compile_definitions(hsl PRIVATE -DHSL_STANDALONE_REPL)
  endif()
endif()

# HSL compiler
if(BUILD_HSLC_PROGRAM)
  add_executable(hslc ${HSL_MAIN_SOURCES} ${HSL_COMPILER_SOURCES} source/Main.cpp source/EntryPoint.cpp)

  target_compile_definitions(hslc PRIVATE -DHSL_COMPILER -DNO_SCRIPT_COMPILING -DTARGET_NAME=\"hslc\")
  if(DEFINED GIT_COMMIT_HASH)
    target_compile_definitions(hslc PRIVATE "-DGIT_COMMIT_HASH=\"${GIT_COMMIT_HASH}\"")
  endif()
  target_include_directories(hslc PUBLIC
    source
    third_party
  )
endif()
