@echo off

set CMAKE="%PROGRAMFILES%\CMake 2.8\bin\cmake.exe"
call "%VS90COMNTOOLS%vsvars32.bat"

if not exist ..\cdash\NUL md cdash
cd ..\cdash

%CMAKE% -DCMAKE_BUILD_TYPE=Release -DCMAKE_BUILD_TYPE=Debug -G "NMake Makefiles" ..

nmake Continuous
