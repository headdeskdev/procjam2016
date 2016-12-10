/**
TODO: explanation
*/

#ifndef PLATFORM_H
#define PLATFORM_H

#include "types.h"
#include "mmath.h"

#define UPDATE_AND_RENDER(name) void name(platform_MainMemory *memory, platform_Input *input, platform_Functions* functions)

#define uint32FromPointer(pointer) ((U32)(size_t)(pointer))
#define pointerFromUint32(type, value) (type *)((size_t)value)

struct platform_MainMemory {
  void* allocatedMemory;
  U32 allocatedMemorySize;
  bool isInitialised;
};

struct platform_FileInfo {
  U32 size;
  void* data;
};
 
#define GET_FILE_SIZE(name) U32 name(char* filename)
#define READ_ENTIRE_FILE(name) platform_FileInfo name(char* filename, void* buffer, U32 bufferLength)
#define WRITE_ENTIRE_FILE(name) void name(char* filename, void* buffer, U32 bufferLength)
#define GET_CLOCK_TIME(name) I64 name()

typedef GET_FILE_SIZE(GetFileSizeFunc);
typedef READ_ENTIRE_FILE(ReadEntireFile);
typedef WRITE_ENTIRE_FILE(WriteEntireFile);
typedef GET_CLOCK_TIME(GetClockTime);

// These are specifically non-renderer functionality required by the engine of the OS
struct platform_Functions {
  GetFileSizeFunc* getFileSize;
  ReadEntireFile* readEntireFile;
  WriteEntireFile* writeEntireFile;
  GetClockTime* getClockTime; // TODO: should be get unix time
};

#include "input.h"

typedef UPDATE_AND_RENDER(UpdateAndRender);

#define CORE_LOOP UPDATE_AND_RENDER(updateAndRender)
#define BEGIN_CORE_LOOP() setPlatformFunctions(functions)

#ifdef MAIN_FILE
#include "platform_functions.cpp"
#endif

#endif