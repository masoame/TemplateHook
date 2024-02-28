//包含用于调试头文件
#include"DebugHook.h"

int main()
{
	//auto temp = DebugHook::SearchProcess(L"ASTLIBRA.exe");
	//if (temp)
	//	DebugHook::InjectDll(temp->th32ProcessID, L"DllHook.dll");
	LoadLibraryA("DllHook.dll");
	while (true)
	{
		MessageBoxW(NULL, L"", L"", MB_OK);
	}
	system("pause");
}