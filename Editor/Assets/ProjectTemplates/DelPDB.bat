@echo off
set projDir=%1%
cd %projDir%/Build/Debug/
del *.pdb
cd %projDir%/Build/RelWithDebInfo/
del *.pdb
cd %projDir%/Build/Release/
del *.pdb