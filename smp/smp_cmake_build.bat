pushd %~dp0

pushd ..\bin
set CC_DIR=%CD%
popd

if not exist build mkdir build

cd build

del /s /q *.* >NUL

if "%DMC_DIR%"=="" set DMC_DIR=c:\dmc

@set "PATH=%CC_DIR%;%DMC_DIR%\bin;%PATH%"

set CC=%CC_DIR%\dmc-cc.exe
set CXX=%CC_DIR%\dmc-cc.exe

:: cmake --trace-expand --debug-output --debug-trycompile -G "MinGW Makefiles" -DCMAKE_C_COMPILER=dmc-cc -DCMAKE_CXX_COMPILER=dmc-cc ..
cmake -G "Unix Makefiles" ..

make

hello

popd
