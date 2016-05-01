@echo off
if not defined DevEnvDir (
	call "E:\Programs\MicrosoftVisualStudio14.0\VC\vcvarsall.bat" x86
)
