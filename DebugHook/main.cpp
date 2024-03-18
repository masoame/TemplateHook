#include"DebugHook.h"
#include"Pipe.h"
int main()
{
	auto a = DebugHook::SearchProcess(L"Start.exe");
	if(a)
	DebugHook::InjectDll(a->th32ProcessID, L"DllHook.dll");
	system("pause");
}