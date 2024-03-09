#include"DebugHook.h"

int main()
{
	LoadLibraryA("DllHook.dll");
	std::cout << "gfhgfhg\n";
	system("pause");
}