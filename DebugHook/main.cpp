//�������ڵ���ͷ�ļ�
#include"DebugHook.h"

int main()
{
	LoadLibraryA("DllHook.dll");
	while (true)
	{
		std::thread([]
			{
				std::cout << "�����߳�" << GetCurrentThreadId() << std::endl;
			}).detach();
		system("pause");
	}
	
}