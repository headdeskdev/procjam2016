//#define ARENA_GET_LIST(Arena,StructName,n) {ARENA_GET_ARRAY(Arena,StructName,n),n}
#include "mmemory.h"

memory_Arena memory_createArena(void* memory, U32 size) {
  memory_Arena arena;
  arena.initMemoryAddress = memory;
  arena.currentMemoryAddress = memory;
  arena.totalSize = size;
  return arena;
};

