@echo off

set projDir=%1
set projName=%2
set buildConfig=%3

echo Compiling %projName%...

@echo off
MSBuild %projDir%Build/%projName%.sln /t:Build /p:Configuration=%buildConfig%

@echo off

exit
