//包含用于调试头文件
#include"DebugHook.h"

int main(int argc, wchar_t* argv[], wchar_t* envp[])
{
	auto a = DebugHook::SearchProcess(L"NUKITASHI2.EXE");
	auto b = DebugHook::InjectDll(a->th32ProcessID, L"DllHook.dll");
	system("pause");
	return 0;
}