@echo OFF
REM Simple one-click batch file for executing a Debug, x64 build using Visual
REM Studio 2013 aka vc12 compiler. Note that cmake.exe and git.exe must be part
REM of %PATH%.

REM load environment for Visual Studio 2013
call "%VS120COMNTOOLS%"vsvars32.bat

REM do initial configuration if required
if not exist build (
  mkdir build
  cd /D build
  cmake .. -G "Visual Studio 12 Win64"
) else (
  cd /D build
)

msbuild /p:Configuration=Debug;Platform=x64 /m ALL_BUILD.vcxproj
pause
