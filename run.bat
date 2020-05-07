@echo off
cls
echo starting compile ...
make debug -j2
if %errorlevel%==0 (
	echo finished !
	codeSync.exe tests
) else (
	echo failed to compile
)
