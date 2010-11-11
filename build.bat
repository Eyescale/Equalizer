@echo off

set CMAKE="%PROGRAMFILES%\CMake 2.8\bin\cmake.exe"
set SUBWCREV="%PROGRAMFILES%\TortoiseSVN\bin\SubWCRev.exe"
set DESTINATION="C:\Equalizer"

call "%VS90COMNTOOLS%vsvars32.bat"

if not exist build\NUL md build
cd build

%CMAKE% -DCMAKE_BUILD_TYPE=Release -G "NMake Makefiles" ..

nmake preinstall

%CMAKE% -DCOMPONENT=Unspecified -DCMAKE_INSTALL_PREFIX=%DESTINATION% -P cmake_install.cmake
%CMAKE% -DCOMPONENT=dev         -DCMAKE_INSTALL_PREFIX=%DESTINATION% -P cmake_install.cmake
%CMAKE% -DCOMPONENT=tools       -DCMAKE_INSTALL_PREFIX=%DESTINATION% -P cmake_install.cmake
%CMAKE% -DCOMPONENT=vmmlib      -DCMAKE_INSTALL_PREFIX=%DESTINATION% -P cmake_install.cmake

%SUBWCREV% .. > "%DESTINATION%\include\eq\Version.txt"

pause
