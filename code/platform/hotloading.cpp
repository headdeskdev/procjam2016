// Hotloading inspired by handmade hero
#ifndef NOHOTLOADING

static struct EXEState {
  char fileFullPath[MAX_PATH];
  char *fileName;
} exeState;

struct platform_AppCode {
    HMODULE appCodeDLL;
    FILETIME lastWriteTimeDLL;

    UpdateAndRender *updateAndRender;
   
    bool isValid;
};

static platform_AppCode code;

inline FILETIME platform_getLastWriteTime(char *filename)
{
    FILETIME lastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA data;
    if(GetFileAttributesEx(filename, GetFileExInfoStandard, &data))
    {
        lastWriteTime = data.ftLastWriteTime;
    }

    return lastWriteTime;
}

static void platform_unloadCode()
{
    if(code.appCodeDLL)
    {
        FreeLibrary(code.appCodeDLL);
        code.appCodeDLL = 0;
    }

    code.isValid = false;
    code.updateAndRender = 0;          
}

static void platform_concatStrings(U32 stringLengthA, char *stringA,
           U32 stringLengthB, char *stringB,
           U32 destStringLength, char *dest)
{
    
    for(int i = 0;
        i < stringLengthA;
        ++i)
    {
        *dest++ = *stringA++;
    }

    for(int i = 0;
        i < stringLengthB;
        ++i)
    {
        *dest++ = *stringB++;
    }

    *dest++ = 0;
}


static int platform_getStringLength(char *string)
{
    int length = 0;
    while(*string++)
    {
        ++length;
    }
    return length;
}

static void platform_buildFullEXEPath(EXEState* state, char *fileName, int destCount, char *dest) {
    platform_concatStrings(state->fileName - state->fileFullPath, state->fileFullPath,
               platform_getStringLength(fileName), fileName, destCount, dest);
} 

static char sourceDLLName[MAX_PATH];
static char tempDLLName[MAX_PATH];
static char lockFileName[MAX_PATH];

static void platform_loadCode()
{
    platform_AppCode result = {};

    WIN32_FILE_ATTRIBUTE_DATA ignored;
    if(!GetFileAttributesEx(lockFileName, GetFileExInfoStandard, &ignored))
    {
        result.lastWriteTimeDLL = platform_getLastWriteTime(sourceDLLName);

        CopyFile(sourceDLLName, tempDLLName, FALSE);
    
        result.appCodeDLL = LoadLibraryA(tempDLLName);
        if(result.appCodeDLL)
        {
            result.updateAndRender = (UpdateAndRender *)
                GetProcAddress(result.appCodeDLL, "updateAndRender");

            result.isValid = (result.updateAndRender != 0);
        }
    }

    if(!result.isValid)
    {
        result.updateAndRender = 0;          
    }

    code = result;
}

// Functions that can be swapped if not building with hotloading on

static void platform_initLoadCode() {
    GetModuleFileNameA(0, exeState.fileFullPath, sizeof(exeState.fileFullPath));    
    exeState.fileName = exeState.fileFullPath;
    for(char *c = exeState.fileFullPath;
        *c;
        ++c)
    {
        if(*c == '\\')
        {
            exeState.fileName = c + 1;
        }
    }

    platform_buildFullEXEPath(&exeState, "_main.dll", MAX_PATH, sourceDLLName);
    platform_buildFullEXEPath(&exeState, "_main_temp.dll", MAX_PATH, tempDLLName);
    platform_buildFullEXEPath(&exeState, "lock.tmp", MAX_PATH, lockFileName);
    platform_loadCode();
}

static bool platform_reloadCode() {
    FILETIME newDLLWriteTime = platform_getLastWriteTime(sourceDLLName);    
    if(CompareFileTime(&newDLLWriteTime, &code.lastWriteTimeDLL) != 0)
    {
        platform_unloadCode();
        platform_loadCode();
        return true;
    }
    return false;
}


static inline UPDATE_AND_RENDER(updateAndRender) {    
    if(code.updateAndRender) {
        code.updateAndRender(memory, input, functions);
    }
}

#else

static void platform_initLoadCode() {    

}

static bool platform_reloadCode() {
    return false;
}

#endif