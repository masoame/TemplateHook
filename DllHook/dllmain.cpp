#include"DllHook.h"
#include"registry.h"
using namespace DllHook;
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:

		RegisterHook::global_context.SetDr7Bits(DebugRegister::LG0, 0b01);
		RegisterHook::global_context.Dr0 = (AUTOWORD)MessageBoxW;
		RegisterHook::global_context.fc[0] = [](_EXCEPTION_POINTERS* info)
			{
				std::thread([thrid = GetCurrentThreadId()] {
					Sleep(0);
					std::cout << "触发一次" << std::endl;
					RegisterHook::AddThreadDebug(thrid);
					}).detach();
				info->ContextRecord->Dr7 = 0;
				return (LONG)EXCEPTION_CONTINUE_EXECUTION;
			};

		break;
	case DLL_THREAD_ATTACH:
	{
		RegisterHook::AddThreadDebug(GetCurrentThreadId());
		
	}
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