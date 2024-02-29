#include"DllHook.h"
#include"registry.h"
using namespace DllHook;

INT3Hook headmomey((LPVOID)0x0043478c, [](_EXCEPTION_POINTERS* info)
	{
		std::thread([]
			{
				Sleep(0);
				headmomey.Hook();
			}).detach();
			info->ContextRecord->Edx *= 100;
			headmomey.UnHook();
		return (LONG)EXCEPTION_CONTINUE_EXECUTION;
	});
INT3Hook plantmomey((LPVOID)0x00468b55, [](_EXCEPTION_POINTERS* info)
	{
		std::thread([]
			{
				Sleep(0);
				plantmomey.Hook();
			}).detach();
			info->ContextRecord->Ecx *= 100;
			plantmomey.UnHook();
			return (LONG)EXCEPTION_CONTINUE_EXECUTION;
	});
INT3Hook addsun((LPVOID)0x0041F4D0, [](_EXCEPTION_POINTERS* info)
	{
		std::thread([]
			{
				Sleep(0);
				addsun.Hook();
			}).detach();
			info->ContextRecord->Ecx *= 10;
			addsun.UnHook();
			return (LONG)EXCEPTION_CONTINUE_EXECUTION;
	});

INT3Hook killzb((LPVOID)0x00545E04, [](_EXCEPTION_POINTERS* info)
	{
		std::thread([]
			{
				Sleep(0);
				killzb.Hook();
			}).detach();
			info->ContextRecord->Ebp = 0;
			killzb.UnHook();
			return (LONG)EXCEPTION_CONTINUE_EXECUTION;
	});
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		headmomey.Hook();
		addsun.Hook();
		plantmomey.Hook();
		killzb.Hook();
		break;
	case DLL_THREAD_ATTACH:
		if (RegisterHook::Insert_ThreadDebug(GetCurrentThreadId()))
			std::cout << GetCurrentThreadId() << "线程打入成功" << std::endl;

		break;
	case DLL_THREAD_DETACH:

	{
		std::unique_lock lock(RegisterHook::mtx, std::try_to_lock);
		RegisterHook::ThrIdToRegister.erase(GetCurrentThreadId());
	}

		break;
	case DLL_PROCESS_DETACH:

		DllHook::INT3Hook::INT3HookStartThread.join();
		DllHook::RegisterHook::RegisterHookStartThread.join();

		break;
	}
	return TRUE;
}