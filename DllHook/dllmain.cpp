#include"DllHook.h"
#include"registry.h"
#include"net.h"

DllHook::INT3Hook test(WriteFile,[](_EXCEPTION_POINTERS* info)
	{
		
		std::thread([]
			{
				Sleep(0);
				MessageBoxA(NULL, "success", "", MB_OK);
				test.Hook();
			}).detach();
		test.UnHook();
		return LONG(EXCEPTION_CONTINUE_EXECUTION);
	} );

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			bool a = test.Hook();
			if (a)
				MessageBoxA(NULL, "success", "", MB_OK);
			else
				MessageBoxA(NULL, "failed", "", MB_ICONERROR);
		}

	break;
	case DLL_THREAD_ATTACH:

		break;
	case DLL_THREAD_DETACH:

		break;
	case DLL_PROCESS_DETACH:

		DllHook::INT3Hook::INT3HookStartThread.join();
		DllHook::RegisterHook::RegisterHookStartThread.join();
		net::StartClient.join();

		break;
	}
	return TRUE;
}