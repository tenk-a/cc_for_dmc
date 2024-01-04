pushd %~dp0

pushd ..\bin
set COMPILER_DIR=%CD%
popd

if not exist build mkdir build

cd build

@set "PATH=%COMPILER_DIR%;c:\dmc\bin;%PATH%"

set CC=%COMPILER_DIR%\cc.exe
set CXX=%COMPILER_DIR%\cc.exe

:: cmake -G "MinGW32 Makefiles" -DCMAKE_C_COMPILER=cc -DCMAKE_CXX_COMPILER=cc ..
cmake -G "Unix Makefiles" ..

popd
