//�������ڵ���ͷ�ļ�
#include"DebugHook.h"

int main()
{
	auto temp = DebugHook::SearchProcess(L"ASTLIBRA.exe");
	if (temp)
		DebugHook::InjectDll(temp->th32ProcessID, L"DllHook.dll");

	system("pause");
}