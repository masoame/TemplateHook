#include"DllHook.h"

namespace DllHook
{
#ifndef _WIN64
	std::unique_ptr<DWORD32[]> GetFuncArgs(const CONTEXT* ct, const DWORD32 argc)
	{
		if (ct == nullptr) return nullptr;

		std::unique_ptr<DWORD32[]> args(new  DWORD32[argc + 1]);

		for (DWORD32 i = 0; i != argc; i++)
		{
			args[i] = *(DWORD32*)(ct->Esp + 4 * i);
		}
		return args;
	}
#else
	std::unique_ptr<DWORD64[]> GetFuncArgs(const CONTEXT* ct, const DWORD64 argc)
	{
		if (ct == nullptr) return nullptr;

		std::unique_ptr<DWORD64[]> args(new  DWORD64[argc + 1 + (argc < 5 ? argc : 4) * 2]);

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
	std::unique_ptr<std::stringstream> GetImportDirectory(const HMODULE hModule)
	{
		if (hModule == nullptr)return nullptr;

		PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)hModule;
		if (DosHeader->e_magic != 0x5A4D) return nullptr;
		IMAGE_NT_HEADERS* NtHeader = (IMAGE_NT_HEADERS*)((DWORD64)hModule + DosHeader->e_lfanew);
		if (NtHeader->Signature != 0x4550) return nullptr;

		IMAGE_EXPORT_DIRECTORY* ExpDir = (IMAGE_EXPORT_DIRECTORY*)((DWORD64)hModule + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

		return std::unique_ptr<std::stringstream>();
	}
	std::unique_ptr<std::stringstream> GetExportDirectory(const HMODULE hModule)
	{
		if (hModule == nullptr)return nullptr;

		PIMAGE_DOS_HEADER DosHeader = (PIMAGE_DOS_HEADER)hModule;
		if (DosHeader->e_magic != 0x5A4D) return nullptr;
		IMAGE_NT_HEADERS* NtHeader = (IMAGE_NT_HEADERS*)((DWORD64)hModule + DosHeader->e_lfanew);
		if (NtHeader->Signature != 0x4550) return nullptr;

		PIMAGE_EXPORT_DIRECTORY ExpDir = (PIMAGE_EXPORT_DIRECTORY)((DWORD64)hModule + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);

		if (ExpDir->AddressOfNameOrdinals == NULL) return nullptr;
		int16_t* OrdinalDir = (int16_t*)((DWORD64)hModule + ExpDir->AddressOfNameOrdinals);
		DWORD32* NameDir = (DWORD32*)((DWORD64)hModule + ExpDir->AddressOfNames);
		DWORD32* FunctionDir = (DWORD32*)((DWORD64)hModule + ExpDir->AddressOfFunctions);
		std::unique_ptr<std::stringstream> ExpDirMsg(new std::stringstream);

		for (int i = 0; i != ExpDir->NumberOfNames; i++)
			*ExpDirMsg << "Address: " << (LPVOID)FunctionDir[OrdinalDir[i]] << " " << (LPCSTR)((DWORD64)hModule + NameDir[i]) << std::endl;	
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------
	std::map<LPVOID, PVECTORED_EXCEPTION_HANDLER> INT3Hook::tb;
	std::mutex INT3Hook::tb_m;
	LPVOID INT3Hook::HandleVEH;

	std::thread INT3Hook::INT3HookStartThread([]
		{
			std::cout << "VEH_ThreadId: " << GetCurrentThreadId() << std::endl;

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
		});

	INT3Hook::INT3Hook(LPVOID Address, PVECTORED_EXCEPTION_HANDLER backcall)
	{
		this->Address = (LPBYTE)Address;
		this->Original = NULL;
		if (this->Address == nullptr) return;

		DWORD temp;
		VirtualProtect(this->Address, 1, PAGE_EXECUTE_READWRITE, &temp);
		if (Address && backcall)
		{
			tb_m.lock();
			tb[Address] = backcall;
			tb_m.unlock();
		}
	}
	INT3Hook::~INT3Hook()
	{
		if (this->Address == nullptr) return;

		std::lock_guard<std::mutex> lg(INT3Hook::tb_m);
		UnHook();
	}
	BOOL INT3Hook::Hook(LPVOID Address, PVECTORED_EXCEPTION_HANDLER backcall)
	{
		std::lock_guard<std::mutex> lg(INT3Hook::tb_m);

		if (!Address && !this->Address) return false;
		else if (Address) this->Address = (LPBYTE)Address;

		if (backcall) tb[this->Address] = backcall;
		if (tb.find(this->Address) == tb.end()) return false;

		DWORD temp;
		VirtualProtect(this->Address, 1, PAGE_EXECUTE_READWRITE, &temp);

		if (*this->Address == 0xCC) return true;
		this->Original = *this->Address;
		*this->Address = 0xCC;

		return true;
	}

	inline BOOL INT3Hook::UnHook() const
	{
		*this->Address = this->Original;
		return 0;
	}

	//-----------------------------------------------------------------------------------------------------------------------

	std::mutex tb_m;
	std::map<LPVOID, PVECTORED_EXCEPTION_HANDLER> tb;
	LPVOID HandleVEH;

	std::thread RegisterHookStartThread([]
		{

		});

	RegisterHook::RegisterHook()
	{

	}
	RegisterHook::~RegisterHook()
	{

	}
	BOOL RegisterHook::Hook(LPVOID* Address)
	{

		return 0;
	}
	BOOL RegisterHook::UnHook()
	{

		return 0;
	}
}