#include"DllHook.h"
#include"test.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//headmomey.Hook();
		//addsun.Hook();
		//plantmomey.Hook();
		//killzb.Hook();
		break;
	case DLL_THREAD_ATTACH:
		if (DllHook::RegisterHook::Insert_ThreadDebug(GetCurrentThreadId()))
			std::cout << GetCurrentThreadId() << "线程打入成功" << std::endl;
		break;
	case DLL_THREAD_DETACH:
	{
		std::unique_lock lock(DllHook::RegisterHook::mtx, std::try_to_lock);
		DllHook::RegisterHook::ThrIdToRegister.erase(GetCurrentThreadId());
	}
		break;
	case DLL_PROCESS_DETACH:

		DllHook::INT3Hook::INT3HookStartThread.join();
		DllHook::RegisterHook::RegisterHookStartThread.join();

		break;
	}
	return TRUE;
}