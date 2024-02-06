#include"DllHook.h"

DllHook::INT3Hook INT3WriteFile((LPVOID)WriteFile);

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		INT3WriteFile.Hook(nullptr, [](_EXCEPTION_POINTERS* info)
			{
				auto args = DllHook::GetFuncArgs(info->ContextRecord, 5);
				std::thread([args, t_info = *info->ContextRecord]
					{
						std::stringstream str1;
						str1 << "Rip值：" <<

							MessageBoxA(NULL, str1.str().c_str(), "函数调用信息", MB_OK);

						Sleep(0);
						INT3WriteFile.Hook();
					}).detach();

					INT3WriteFile.UnHook();
					return (LONG)EXCEPTION_CONTINUE_EXECUTION;
			});
		break;
	case DLL_THREAD_ATTACH:
		//std::cout << "dll线程创建" << std::endl;

		break;
	case DLL_THREAD_DETACH:
		//std::cout << "dll线程销毁" << std::endl;
		break;
	case DLL_PROCESS_DETACH:

		break;
	}
	return TRUE;
}