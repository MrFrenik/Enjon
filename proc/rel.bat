@echo off
pushd bin
cl /Zi /FS /Gm /Ox /Femain.exe ^
..\src\*.cpp ..\src\Graphics\*.cpp ..\src\IO\*.cpp ..\src\Math\*.cpp  ..\src\Physics\*.cpp ..\src\Utils\*.cpp ^
..\IsoARPG\src\*.cpp ..\IsoARPG\src\ECS\*.cpp ^
/I ..\include /I ..\deps\include /I ..\IsoARPG\include /I ..\IsoARPG\include\ECS ^
/EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib /LIBPATH:"..\deps\lib\" Opengl32.lib SDL2.lib SDL2main.lib glew32.lib glew32s.lib freetype.lib
popd
