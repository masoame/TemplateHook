#include"DllHook.h"
#include"registry.h"
#include"net.h"


BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		DllHook::RegisterHook::global_context.Dr0 = (DllHook::AUTOWORD)MessageBoxA;
		DllHook::RegisterHook::global_context.Dr7.SetBits(DllHook::DebugAddressRegister::__Dr7::LG0, 0b01);
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