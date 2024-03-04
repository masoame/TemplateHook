@echo off
cd DebugHook
g++ -w DebugHook.cpp main.cpp -o ../g++build/DebugHook.exe
cd ..
echo DebugHook.exe build sucess!!!
cd DllHook
g++ -w -shared DllHook.cpp registry.cpp test.cpp dllmain.cpp -o ../g++build/DllHook.dll
cd ..
echo DllHook.dll build sucess!!!
pause