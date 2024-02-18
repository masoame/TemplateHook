//包含用于调试头文件
#include"DebugHook.h"

int main(int argc, wchar_t* argv[], wchar_t* envp[])
{
	LoadLibrary(L"DllHook");
	std::cout << "主线程id: " << GetCurrentThreadId() << std::endl;

	system("pause");
	CONTEXT temp;
	temp.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(GetCurrentThread(),&temp);


	while (true)
	{
		system("pause");
		MessageBoxA(NULL, "触发", "", MB_OK);
	}
	system("pause");

	return 0;
}