//包含用于调试头文件
#include"DebugHook.h"



int main(int argc, wchar_t* argv[], wchar_t* envp[])
{
	LoadLibraryA("DllHook.dll");
	system("pause");
	return 0;
}