@echo off
pushd bin
cl -Zi -Gm /Femain.exe ..\src\*.cpp ..\src\Math\*.cpp ..\src\Graphics\*.cpp ..\IsoARPG\src\*.cpp ..\IsoARPG\src\ECS\*.cpp /I ..\include /I ..\deps\include /I ..\IsoARPG\include /I ..\IsoARPG\include\ECS /EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /LIBPATH:"..\deps\lib\" Opengl32.lib SDL2.lib SDL2main.lib glew32.lib glew32s.lib
popd
