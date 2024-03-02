//包含用于调试头文件
#include"DebugHook.h"

int main()
{

	LoadLibraryA("DllHook.dll");
	//auto a = DebugHook::SearchProcess(L"popcapgame1.exe");
	//if (a)
	//	DebugHook::InjectDll(a->th32ProcessID, L"DllHook.dll");
	//auto a = DebugHook::LoadFile(L"D:\\games\\steamapps\\common\\Plants Vs Zombies\\PlantsVsZombies.exe");
	//auto b = DebugHook::RVAtoFOA(a.get(), 0x145E04);
	//std::cout << (LPVOID)b << std::endl;


	//DebugHook::ShowImportTable(GetCurrentProcess(), GetModuleHandleA("DebugHook.exe"));

	//std::thread([]
	//	{
	//		while (!IsDebuggerPresent()) Sleep(100);

	//		ExitProcess(-1);
	//	}).detach();
	system("pause");
}