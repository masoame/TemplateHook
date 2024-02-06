#include"DllHook.h"

namespace DllHook
{
#ifndef _WIN64
	std::shared_ptr<DWORD32[]> GetFuncArgs(CONTEXT* ct, DWORD32 argc)
	{
		if (ct == nullptr) return nullptr;

		std::shared_ptr<DWORD32[]> args(new  DWORD32[argc + 1]);

		for (DWORD32 i = 0; i != argc; i++)
		{
			args[i] = *(DWORD32*)(ct->Esp + 4 * i);
		}
		return args;
	}
#else
	std::shared_ptr<DWORD64[]> GetFuncArgs(CONTEXT* ct, DWORD64 argc)
	{
		if (ct == nullptr) return nullptr;

		std::shared_ptr<DWORD64[]> args(new  DWORD64[argc + 1 + (argc % 5) * 2]);

		switch (argc)
		{
		default:
			for (DWORD64 i = argc; i != 4; i--)
			{
				args[i] = *(DWORD64*)(ct->Rsp + 8 * i);
			}
		case 4:
			args[4] = ct->R9; 
			args[argc + 7] = ct->Xmm0.High;
			args[argc + 8] = ct->Xmm0.Low;
		case 3:
			args[3] = ct->R8;
			args[argc + 5] = ct->Xmm0.High;
			args[argc + 6] = ct->Xmm0.Low;
		case 2:
			args[2] = ct->Rdx;
			args[argc + 3] = ct->Xmm0.High;
			args[argc + 4] = ct->Xmm0.Low;
		case 1:
			args[1] = ct->Rcx;
			args[argc + 1] = ct->Xmm0.High;
			args[argc + 2] = ct->Xmm0.Low;
		case 0:
			args[0] = *(DWORD64*)(ct->Rsp);
		}
		return args;
	}
#endif

	std::map<LPVOID, PVEH> INT3Hook::tb;
	std::mutex INT3Hook::tb_m;
	LPVOID INT3Hook::HandleVEH;

	std::thread INT3Hook::INT3HookStratThread([]
		{
			std::cout << "开启VEH..." << std::endl;
			INT3Hook::HandleVEH = AddVectoredExceptionHandler(1, [](_EXCEPTION_POINTERS* ExceptionInfo)
				{
					std::lock_guard<std::mutex> lg(INT3Hook::tb_m);

					if (INT3Hook::tb.find(ExceptionInfo->ExceptionRecord->ExceptionAddress) != INT3Hook::tb.end())
					{
						if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
						{
							return INT3Hook::tb[ExceptionInfo->ExceptionRecord->ExceptionAddress](ExceptionInfo);
						}
					}
					return (LONG)EXCEPTION_CONTINUE_SEARCH;
				});

			if (INT3Hook::HandleVEH) std::cout << "VEH开启成功" << std::endl;
		});

	INT3Hook::INT3Hook(LPVOID Address)
	{
		this->Address = (LPBYTE)Address;
		this->Original = NULL;
	}
	INT3Hook::~INT3Hook()
	{
		std::lock_guard<std::mutex> lg(INT3Hook::tb_m);
		UnHook();
		if (tb.find(this->Address) != tb.end()) tb.erase(this->Address);
	}
	BOOL INT3Hook::Hook(LPVOID Address, PVEH backcall)
	{
		if (!Address && !this->Address) return false;
		else if (Address) this->Address = (LPBYTE)Address;
		this->Original = *this->Address;

		DWORD temp;
		VirtualProtect(this->Address, 1, PAGE_EXECUTE_READWRITE, &temp);
		*this->Address = 0xCC;

		if (backcall)
		{
			std::lock_guard<std::mutex> lg(INT3Hook::tb_m);
			tb[this->Address] = backcall;
		}

		return true;
	}
	BOOL INT3Hook::UnHook()
	{
		*this->Address = this->Original;
		return 0;
	}
}