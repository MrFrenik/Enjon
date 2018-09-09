@echo off
set src=WriteResourceFileMain.cpp
cl /w -Zi /MP /FeWriteResourceFile.exe %src% /EHsc /link /SUBSYSTEM:CONSOLE /NODEFAULTLIB:msvcrt.lib
rm *.pdb
rm *.ilk
rm *.obj
