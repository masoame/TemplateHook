#include"DllHook.h"
#include"registry.h"

DllHook::INT3Hook test(WriteFile, [](_EXCEPTION_POINTERS* info)
	{
		std::thread([]
			{
				Sleep(0);
				MessageBoxA(NULL, "success", "", MB_OK);
				test.Hook();
			}).detach();
			test.UnHook();
			return LONG(EXCEPTION_CONTINUE_EXECUTION);
	});

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		auto a = DllHook::GetExportDirectory(GetModuleHandleA("kernel32.dll"));
		std::cout << a->str() << std::endl;

		std::cout << GetProcAddress(GetModuleHandleA("kernel32.dll"), "WriteFile");
	}

	break;
	case DLL_THREAD_ATTACH:

		break;
	case DLL_THREAD_DETACH:

		break;
	case DLL_PROCESS_DETACH:

		DllHook::INT3Hook::INT3HookStartThread.join();
		DllHook::RegisterHook::RegisterHookStartThread.join();

		break;
	}
	return TRUE;
}