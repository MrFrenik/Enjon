@echo off
if not defined DevEnvDir (
	call "D:\Programs\MicroSoft Visual Studio Community 2013\VC\vcvarsall.bat" x86
)
set path=D:\C++Projects\VS\3DEngine\proc;%path%