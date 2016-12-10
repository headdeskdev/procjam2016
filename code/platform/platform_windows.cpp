#include <windows.h>

// EXTERNAL LIBRARIES
#include <SDL.h>
#include <SDL_mixer.h>

// C LIBRARIES
#include <assert.h>
#include <stdio.h>
#include <string.h>

// CORE PLATFORM DEFINITIONS
#include "../engine/platform.h"

#include "hotloading.cpp"
#include "controller_windows.cpp"
#include "../renderer/graphics.h"

// Declare the core loop function
UPDATE_AND_RENDER(updateAndRender);

static platform_MainMemory mainMemory;  
static SDL_Window *window;
static long long int globalCounterFrequency;

LARGE_INTEGER platform_getWallClock() {
  LARGE_INTEGER Result;
  QueryPerformanceCounter(&Result);
  return(Result);
}

// TODO: add prefix
U32 getFileSize(char* filename) {
    HANDLE file;
    LARGE_INTEGER fileSize;
    file = CreateFile(filename, GENERIC_READ, 0, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);                   
    if (INVALID_HANDLE_VALUE == file) {
        // TODO: handle load error      
        return 0;
    }    
    if (!GetFileSizeEx(file, &fileSize)) {
        // TODO: handle size error
        return 0;
    } else {
	  CloseHandle(file);
        return (U32) fileSize.QuadPart;
    }	  
}

platform_FileInfo readEntireFile(char* filename, void* buffer, U32 bufferSize) {
    HANDLE file;
    LARGE_INTEGER fileSize;
    platform_FileInfo fileInfo;
    platform_FileInfo emptyFile = {0};
    
    file = CreateFile(filename, GENERIC_READ, 0, NULL,
                      OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);                   
    if (INVALID_HANDLE_VALUE == file) {
        // TODO: handle load error      
        return emptyFile;
    }    
    if (!GetFileSizeEx(file, &fileSize)) {
        // TODO: handle size error
        return emptyFile;
    }

    fileInfo.size = mmin(bufferSize, (U32) fileSize.QuadPart);      
    fileInfo.data = buffer;

    DWORD bytesRead;
    if(!ReadFile(file,fileInfo.data,(DWORD) fileInfo.size,&bytesRead,0)) {
      // TODO: handle read error
      return emptyFile;
    }

    CloseHandle(file);
    
    return fileInfo; // Return our string
}

void writeEntireFile(char* filename, void* buffer, U32 bufferSize) {
    HANDLE file;
    LARGE_INTEGER file_size;
    platform_FileInfo fileInfo;
    platform_FileInfo emptyFile = {0};
    
    file = CreateFile(filename, GENERIC_WRITE, 0, NULL,
                      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);                   
    if (INVALID_HANDLE_VALUE == file) {
        // TODO: handle load error      
        return;
    }    

    DWORD bytesWritten;
    WriteFile(file,buffer,(DWORD) bufferSize,&bytesWritten,0);

    CloseHandle(file);    
}


GET_CLOCK_TIME(getClockTime) {
  return platform_getWallClock().QuadPart;  
}

static platform_MainMemory* platform_initPlatform() {

  platform_initLoadCode();
  
  // TODO: how do we vary this memory
  mainMemory.allocatedMemorySize = 268435456;
  mainMemory.allocatedMemory = VirtualAlloc(0, mainMemory.allocatedMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

  mainMemory.isInitialised = false;  
  
  SDL_Window *mainwindow;   
  SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO);
  platform_loadXInput();
  SDL_VideoInit(NULL);   
  
  mainwindow = SDL_CreateWindow("TODO: Fix Title", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      1024, 768, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  window = mainwindow;

  SDL_GLContext maincontext; 
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);   
  maincontext = SDL_GL_CreateContext(window);    
  SDL_GL_SetSwapInterval(1);

  // Mix_Init(MIX_INIT_OGG);
  // int open = Mix_OpenAudio(44100,MIX_DEFAULT_FORMAT,2,1024);
  // Mix_AllocateChannels(16);    

  LARGE_INTEGER performanceCounterFrequency;
  QueryPerformanceFrequency(&performanceCounterFrequency);
  globalCounterFrequency = performanceCounterFrequency.QuadPart;
  timeBeginPeriod(1);

  return &mainMemory;
}

static F32 platform_getSecondsElapsed(LARGE_INTEGER start, LARGE_INTEGER end) {
  return ((F32)(end.QuadPart - start.QuadPart) /
                   (F32)globalCounterFrequency);
}      


// TODO: should these be platform functions
const char* platform_getClipboardText()
{
  return SDL_GetClipboardText();
}

void platform_setClipboardText(const char* text)
{
  SDL_SetClipboardText(text);
}

void platform_setRelativeTouchPosition(platform_PointerState* pointer, U8 transitionIndex, Vector2 windowOffset, Vector2 windowSize, F32 x, F32 y) {
  // TODO: make this work with multiple devices?    
  pointer->x[transitionIndex] = (F32) (x*windowSize.x);// - windowOffset.x;
  pointer->y[transitionIndex] = (F32) (y*windowSize.y);// - windowOffset.y;
}

static struct {
  U32 id[MAX_FINGERS]; 
  bool flag[MAX_FINGERS];  
  U32 count;
} touchIDMap;

I32 platform_getTouchID(U32 fingerID) {
  I32 id = -1;
  I32 availableID = -1;  
  for (int i = 0; i < touchIDMap.count; i++) {
    if (touchIDMap.id[i] == 0xFFFFFFFF && availableID == -1) {
      availableID = i;
    } else if (touchIDMap.id[i] == fingerID) {
      id = i;
      touchIDMap.flag[id] = true;      
    }
  }  
  if (id == -1) {
    if (availableID != -1) {
      id = availableID;
      touchIDMap.id[id] = fingerID;
      touchIDMap.flag[id] = true;  
    } else if (touchIDMap.count < MAX_FINGERS) {
      touchIDMap.id[touchIDMap.count] = fingerID;
      id = touchIDMap.count;
      touchIDMap.flag[id] = true;  
      touchIDMap.count++;    
    }
  }
  return id; 
}

void platform_updateTouchIDMap() {
  for (int i = 0; i < touchIDMap.count; i++) {
    if (!touchIDMap.flag[i]) {
      touchIDMap.id[i] = 0xFFFFFFFF;
    }
    touchIDMap.flag[i] = false;
  }
}

void platform_getInput(platform_Input* input, platform_Input* old) {
  SDL_GetWindowSize(window, &input->windowWidth, &input->windowHeight);
  I32 windowX = 0;
  I32 windowY = 0;
  SDL_GetWindowPosition(window, &windowX, &windowY);
  Vector2 windowOffset = {(F32)windowX,(F32)windowY};
  Vector2 windowSize = {(F32)input->windowWidth, (F32)input->windowHeight};

  SDL_Event e;  
  while (SDL_PollEvent(&e)) {
    switch (e.type) {      
      case SDL_QUIT: {
        input->quit = true;
      } break;
      case SDL_MOUSEWHEEL: {
        if (e.wheel.y > 0)
          input->mouseWheel = 1.0f;
        if (e.wheel.y < 0)
          input->mouseWheel = -1.0f;        
      } break;
      case SDL_MOUSEBUTTONUP:
      case SDL_MOUSEBUTTONDOWN: {
        int mouseIndex = -1;
        if (e.button.button == SDL_BUTTON_LEFT) {mouseIndex = 0;}
        if (e.button.button == SDL_BUTTON_RIGHT) {mouseIndex = 1;}
        if (e.button.button == SDL_BUTTON_MIDDLE) {mouseIndex = 2;}
        if (mouseIndex >= 0) {
          int t = input->mousePointers[mouseIndex].button.halfTransitionCount;
          if (t >= MAX_TRANSITION_COUNT) {
            // TODO: handle over 4 clicks per frame 
          } else {
            input->mousePointers[mouseIndex].x[t] = (F32) e.button.x;  
            input->mousePointers[mouseIndex].y[t] = (F32) e.button.y;  
            input->mousePointers[mouseIndex].button.halfTransitionCount++;
          }         
        }
      } break; 
      case SDL_KEYUP:
      case SDL_KEYDOWN: {
        if (e.key.repeat) {
          input->keyboardButtons[e.key.keysym.scancode].textRepeat = true;
        } else {
          input->keyboardButtons[e.key.keysym.scancode].halfTransitionCount++;  
        }        
      } break;   
      case SDL_TEXTINPUT: {     
        I8 i = 0;
        while (input->textInputSize < 16) {
          input->textInput[input->textInputSize] = e.text.text[i++];
          input->textInputSize++;          
        } 
        input->textInput[input->textInputSize] = 0;                   
      } break;
      case SDL_FINGERDOWN:
      case SDL_FINGERUP: {
        // TODO: ensure coming from correct touch device (in case of 2+ touch devices)
        int touchIndex = e.tfinger.fingerId;
        if (touchIndex >= 0 && touchIndex < MAX_FINGERS) {
          int t = input->touchPointers[touchIndex].button.halfTransitionCount;
          if (t >= MAX_TRANSITION_COUNT) {
            // TODO: handle over 4 clicks per frame 
          } else {
            platform_setRelativeTouchPosition(&input->touchPointers[touchIndex],t,windowOffset,windowSize,e.tfinger.x,e.tfinger.y); 
            input->touchPointers[touchIndex].button.halfTransitionCount++;
          }         
        }
      } break; 
    }
  }
  const U8 *keystate = SDL_GetKeyboardState(NULL);
  for (I32 i = 0; i < 300; i++) {
    input->keyboardButtons[i].isDown = keystate[i] > 0;
  }

  int mx, my;
  U32 mouseMask = SDL_GetMouseState(&mx, &my);
  if (!(SDL_GetWindowFlags(window) & SDL_WINDOW_MOUSE_FOCUS)) {
    mx = -1;
    my = -1;    
  } else {
    for (int i = 0; i < 5; i++) {
      int t = input->mousePointers[i].button.halfTransitionCount;
      input->mousePointers[i].x[t] = (F32) mx;
      input->mousePointers[i].y[t] = (F32) my;
    }
  }   
  input->mousePointers[0].button.isDown = (mouseMask & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;   
  input->mousePointers[1].button.isDown = (mouseMask & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
  input->mousePointers[2].button.isDown = (mouseMask & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
  
  input->getClipboardText = platform_getClipboardText;
  input->setClipboardText = platform_setClipboardText;    
  
  if (SDL_GetNumTouchDevices() > 0) {    
    SDL_TouchID id = SDL_GetTouchDevice(0);
    for (int i = 0; i < SDL_GetNumTouchFingers(id); i++) {
      // TODO: actual handling of mouse + touch
	  platform_MousePointerState mps = {0.0};
      input->_mp = mps;
      SDL_Finger* finger = SDL_GetTouchFinger(id, i);
      I32 inputID = platform_getTouchID(finger->id);
      if (inputID != -1) {
        int t = input->touchPointers[inputID].button.halfTransitionCount;      
        input->touchPointers[inputID].button.isDown = true;
        platform_setRelativeTouchPosition(&input->touchPointers[inputID],t,windowOffset,windowSize,finger->x,finger->y);      
      }      
    }
    platform_updateTouchIDMap();
  }

  if (old->lockMouse) {
    SDL_WarpMouseInWindow( window, input->windowWidth / 2, input->windowHeight / 2);
    SDL_ShowCursor(0);
  } else {
    SDL_ShowCursor(1);
  }

  platform_getControllerInput(&old->controller, &input->controller);
}

int CALLBACK 
WinMain(HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow) 
{
	platform_initPlatform(); 
	platform_Input input = {0};
	
	//TODO: some sort of modifiability for this
	F32 targetFrameDuration = 1.0/60.0;
	F32 frameSeconds = targetFrameDuration;
  platform_Functions functions = {getFileSize, readEntireFile, writeEntireFile, getClockTime};
	
    while(true) {        
        LARGE_INTEGER lastTimeCount = platform_getWallClock();          

        platform_Input newInput = {0};               
        platform_getInput(&newInput, &input);
        input = newInput;

        input.t = frameSeconds;
        input.reload = platform_reloadCode();

        I32 pww = input.windowWidth;
        I32 pwh = input.windowHeight;
        updateAndRender(&mainMemory, &input, &functions); 
        if (input.windowWidth != pww || input.windowHeight != pwh) {
          SDL_SetWindowSize(window, input.windowWidth, input.windowHeight);
        }
		
		Vector2 windowSize = {(F32)input.windowWidth, (F32)input.windowHeight};
        
        
        if (input.quit) {
          return 0;
        } 

        SDL_GL_SwapWindow(window);

        frameSeconds = platform_getSecondsElapsed(lastTimeCount, platform_getWallClock());

        if(frameSeconds < targetFrameDuration) {
			DWORD sleepMilliseconds = (DWORD)(1000.0f * (targetFrameDuration -
											   frameSeconds));
			if(sleepMilliseconds > 0) {
				Sleep(sleepMilliseconds);
			}
                
            while(frameSeconds < targetFrameDuration) {
				frameSeconds = platform_getSecondsElapsed(lastTimeCount, platform_getWallClock());
            }
        }
    }    
    return 0;       
}