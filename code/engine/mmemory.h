#ifndef MMEMORY_H
#define MMEMORY_H

#include "types.h"

#define ARENA_GET_STRUCT(arena,StructName) (StructName*) (arena).get(sizeof(StructName))
#define ARENA_GET_ARRAY(arena,StructName,n) (StructName*) (arena).get(sizeof(StructName)*n)
#define MASSERT(x) ((x) ? 0 : *((I32*) 0));

struct memory_Stack {
    void* init_memory_address;
    void* current_memory_address;
    void* stack_location;
    U32 size;
    void init(U32 p_size, void* memory) {
        size = p_size;
        stack_location = 0;
        init_memory_address = memory;
        current_memory_address = memory;
    }
    void* get(U32 size) {
        void* mem = current_memory_address;
        current_memory_address = (void*) (((U8*)current_memory_address) + size);
        return mem;
    };
    void push() {
        void* previous_stack_location = stack_location;
        stack_location = current_memory_address;
        *(void**)get(sizeof(previous_stack_location)) = previous_stack_location;
    }
    void pop() {
        void* previous_stack_location = *((void**)stack_location);
        current_memory_address = stack_location;
        stack_location = previous_stack_location;
    };
};

struct memory_Arena {
  void* initMemoryAddress;
  void* currentMemoryAddress; // TODO: does address make sense? should it be currentSize
  U32 totalSize;
  void* get(U32 size) {
    // TODO: bounds checking? at least on debug/testing mode?
    void* memory = currentMemoryAddress;
    currentMemoryAddress = (void*) (((U8*)currentMemoryAddress) + size);
    return memory;
  };
  void clear() {
    currentMemoryAddress = initMemoryAddress;
  };
};

template<class T>
struct memory_UnorderedRemoveList {
  T* memory;
  U32 size;
  U32 length;

  inline void init(T* initMemory, U32 initSize) {
    memory = initMemory;
	size = initSize;
  };

  inline void push(T element) {
    MASSERT(length < size);
    memory[length++] = element;
  }

  inline T pop() {
    length--;
    return memory[length];
  }

  inline void remove(U32 index) {
    memory[index] = memory[length-1];
  }

  inline T& operator[](int i) {
    return memory[i];
  }
};

memory_Arena memory_createArena(void* memory, U32 size);

#endif
