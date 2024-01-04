pushd %~dp0

pushd ..\bin
set CC_DIR=%CD%
popd

if not exist build mkdir build

cd build

del /s /q *.* >NUL

if "%DMC%"=="" set DMC=c:\dmc

@set "PATH=%CC_DIR%;%DMC%\bin;%PATH%"

set CC=%CC_DIR%\cc.exe
set CXX=%CC_DIR%\cc.exe

:: cmake --trace-expand --debug-output --debug-trycompile -G "MinGW32 Makefiles" -DCMAKE_C_COMPILER=cc -DCMAKE_CXX_COMPILER=cc ..
cmake -G "Unix Makefiles" ..

make

hello

popd
