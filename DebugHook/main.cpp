//�������ڵ���ͷ�ļ�
#include"DebugHook.h"

int main(int argc, wchar_t* argv[], wchar_t* envp[])
{
	LoadLibrary(L"DllHook");
	std::cout << "���߳�id: " << GetCurrentThreadId() << std::endl;

	system("pause");
	CONTEXT temp;
	temp.ContextFlags = CONTEXT_DEBUG_REGISTERS;
	GetThreadContext(GetCurrentThread(),&temp);


	while (true)
	{
		system("pause");
		MessageBoxA(NULL, "����", "", MB_OK);
	}
	system("pause");

	return 0;
}