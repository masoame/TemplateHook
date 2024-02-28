//包含用于调试头文件
#include"DebugHook.h"

int main()
{
	LoadLibraryA("DllHook.dll");
	while (true)
	{
		std::thread([]
			{
				std::cout << "创建线程" << GetCurrentThreadId() << std::endl;
			}).detach();
		system("pause");
	}
	
}