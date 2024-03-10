#include"DllHook.h"
#include"test.h"
#include"Pipe.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		auto a = DllHook::GetImportDirectory(GetModuleHandleA(nullptr));
		Pipe::pout << a->str();
	}
	break;
	case DLL_THREAD_ATTACH:
		if (DllHook::RegisterHook::Insert_ThreadDebug(GetCurrentThreadId()))
			Pipe::pout << GetCurrentThreadId() << "线程打入成功" << Pipe::pendl;
		break;
	case DLL_THREAD_DETACH:
	{
		std::unique_lock lock(DllHook::RegisterHook::mtx, std::try_to_lock);
		DllHook::RegisterHook::ThrIdToRegister.erase(GetCurrentThreadId());
	}
	break;
	case DLL_PROCESS_DETACH:

		break;
	}
	return TRUE;
}