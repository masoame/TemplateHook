#include"DllHook.h"
#include"registry.h"
#include"net.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		std::wcout.imbue(std::locale(""));
		auto b = registry::GetSoftwareMsg();
		if (b == nullptr)return TRUE;
		std::wcout << b->str();
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