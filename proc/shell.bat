@echo off
if not defined DevEnvDir (
	call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" x86
)
set path=D:\C++Projects\VS\3DEngine\proc;%path%