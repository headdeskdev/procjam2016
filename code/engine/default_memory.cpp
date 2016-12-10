struct DefaultMemory {
    DEFAULT_MEMORY_STATE_STRUCT state;
    memory_Arena memory;
};

DefaultMemory* getDefaultMemory(platform_MainMemory* mainMemory) {
    if (!mainMemory->isInitialised) {
        mainMemory->allocatedMemory;

        memory_Arena allocatedMemoryArena = memory_createArena(mainMemory->allocatedMemory, mainMemory->allocatedMemorySize);
        DefaultMemory* memory = ARENA_GET_STRUCT(allocatedMemoryArena, DefaultMemory);

        // TODO: set up malloc/free to use this memory (requires general purpose allocater + other things)
        
        allocatedMemoryArena.initMemoryAddress = allocatedMemoryArena.currentMemoryAddress;
        
        memory->memory = allocatedMemoryArena;
        return memory;
    } else {
        return (DefaultMemory*) mainMemory->allocatedMemory;
    }
}