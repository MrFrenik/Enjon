@echo off
pushd bin
cl -Zi /Femain.exe ..\src\*.cpp ..\src\Math\*.cpp /I ..\include /I ..\deps\include /EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /LIBPATH:"..\deps\lib\" Opengl32.lib SDL2.lib SDL2main.lib glew32.lib glew32s.lib
popd
