// TODO: should these not be global (especially as static requires reset on reload)

static GetFileSizeFunc* getFileSize;
static ReadEntireFile* readEntireFile;
static WriteEntireFile* writeEntireFile;
static GetClockTime* getClockTime;

void setPlatformFunctions(platform_Functions* functions) {
    getFileSize  = functions->getFileSize;
    readEntireFile = functions->readEntireFile;
    writeEntireFile = functions->writeEntireFile;
    getClockTime = functions->getClockTime;
}

void* initState(platform_MainState* mainState, U32 sizeOfState) {
    if (!mainState->isInitialised) {
        void* memory = malloc(sizeOfState);
        memset(memory, 0, sizeOfState);
        mainState->state = memory;
        mainState->isInitialised = true;
    }
    return mainState->state;
}