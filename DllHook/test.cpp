#include"test.h"

#ifndef _WIN64
DllHook::INT3Hook headmomey((LPVOID)0x0043478c, [](_EXCEPTION_POINTERS* info)
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
DllHook::INT3Hook plantmomey((LPVOID)0x00468b55, [](_EXCEPTION_POINTERS* info)
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

DllHook::INT3Hook addsun((LPVOID)0x0041F4D0, [](_EXCEPTION_POINTERS* info)
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

DllHook::INT3Hook killzb((LPVOID)0x00545E04, [](_EXCEPTION_POINTERS* info)
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
DllHook::INT3Hook killzb1((LPVOID)0x00545B1A, [](_EXCEPTION_POINTERS* info)
	{
		std::thread([]
			{
				Sleep(0);
				killzb1.Hook();
			}).detach();
			info->ContextRecord->Ecx = 0;
			killzb1.UnHook();
			return (LONG)EXCEPTION_CONTINUE_EXECUTION;
	});




#else

void test()
{


}

#endif