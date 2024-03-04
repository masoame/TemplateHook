g++ -w -O3 DebugHook/DebugHook.cpp DebugHook/main.cpp -o DebugHook.exe
g++ -w -O3 -shared DllHook/DllHook.cpp DllHook/registry.cpp DllHook/test.cpp DllHook/dllmain.cpp -o DllHook.dll