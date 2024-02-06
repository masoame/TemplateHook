//包含用于调试头文件
#include"DebugHook.h"

int main(int argc, wchar_t* argv[], wchar_t* envp[])
{
	auto a = DebugHook::SearchProcess(L"notepad++.exe");

	if (a.get() == nullptr)
	{
		std::cout << "未找到程序" << std::endl;
		return -1;
	}
	DebugHook::InjectDll(a->th32ProcessID, L"DllHook");

	return 0;
}