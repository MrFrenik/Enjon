@echo off

set projDir=%1
set projName=%2
set buildConfig=%3

echo Compiling Basic...

@echo off
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" C:\Users\mrfre\Documents\Development\Enjon\Samples\Basic/Build//Basic.sln /t:Build /p:Configuration=Debug

@echo off

exit
