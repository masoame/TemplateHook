#include"DebugHook.h"

int main()
{
	auto a = DebugHook::SearchProcess(L"Test.exe");
	if(a)
	DebugHook::InjectDll(a->th32ProcessID, L"DllHook.dll");
}