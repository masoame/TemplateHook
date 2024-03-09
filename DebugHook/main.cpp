#include"DebugHook.h"

int main()
{
	LoadLibraryA("DllHook.dll");
	std::cout << "test";
	system("pause");
}