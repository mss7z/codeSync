@echo off
cls
echo starting compile ...
make debug -j2
if %errorlevel%==0 (
	echo finished !
	codeSync.exe D:\00_C_Cpp_Program\codeShare\tests
) else (
	echo failed to compile
)
