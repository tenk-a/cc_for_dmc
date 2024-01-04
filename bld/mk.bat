setlocal
pushd %~dp0

if "%DMC%"=="" set DMC=c:\dmc

dmc -I%DMC%\stlport\stlport -DNDEBUG -o+all -o..\bin\cc.exe ..\src\cc.cpp

del *.bak *.obj *.map

popd
endlocal
