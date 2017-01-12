@echo off

set LIBS=libcmt.lib OpenGL32.lib
set PLATFORM_LIBS=SDL2.lib SDL2main.lib OpenGL32.lib user32.lib gdi32.lib winmm.lib kernel32.lib Advapi32.lib libcmt.lib SDL2_mixer.lib
set COMPILE_OPTIONS=/O2
set LINK_OPTIONS=/incremental:no /opt:ref /LIBPATH:"..\external\lib" /NODEFAULTLIB:LIBCMT


IF NOT EXIST ..\build mkdir ..\build
pushd ..\build

del *.pdb
cl /I..\external\include ..\code\main.cpp -Fm_main.map %COMPILE_OPTIONS% /Fe_main.dll -LD %LIBS% /link %LINK_OPTIONS% -PDB:_main_%random%.pdb -EXPORT:updateAndRender
del lock.tmp
cl /I..\external\include ..\code\platform\platform_windows.cpp /FmFeprocjam2016.map %COMPILE_OPTIONS% /Feprocjam2016.exe %PLATFORM_LIBS% /link %LINK_OPTIONS%

popd
