@echo off
pushd bin
cl /MP /FS /Ox /w /Fegen.exe ^
..\generator\*.cpp ^
/I ..\generator\*.h ^
/EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib 
popd bin