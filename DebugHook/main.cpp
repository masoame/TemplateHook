//�������ڵ���ͷ�ļ�
#include"DebugHook.h"



int main(int argc, wchar_t* argv[], wchar_t* envp[])
{
	LoadLibraryA("DllHook.dll");

	return 0;
}