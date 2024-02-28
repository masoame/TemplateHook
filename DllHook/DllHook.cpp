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
			args[argc + 7] = ct->Xmm0.Low;
			args[argc + 8] = ct->Xmm0.High;
		case 3:
			args[3] = ct->R8;
			args[argc + 5] = ct->Xmm0.Low;
			args[argc + 6] = ct->Xmm0.High;
		case 2:
			args[2] = ct->Rdx;
			args[argc + 3] = ct->Xmm0.Low;
			args[argc + 4] = ct->Xmm0.High;
		case 1:
			args[1] = ct->Rcx;
			args[argc + 1] = ct->Xmm0.Low;
			args[argc + 2] = ct->Xmm0.High;
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
		PIMAGE_NT_HEADERS NtHeader = (PIMAGE_NT_HEADERS)((AUTOWORD)hModule + DosHeader->e_lfanew);
		if (NtHeader->Signature != 0x4550) return nullptr;

		PIMAGE_IMPORT_DESCRIPTOR ImpDir = (PIMAGE_IMPORT_DESCRIPTOR)((AUTOWORD)hModule + NtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
		if (ImpDir->Characteristics == 0) return nullptr;
		PIMAGE_IMPORT_BY_NAME IBN;
		PIMAGE_THUNK_DATA _IAT = (PIMAGE_THUNK_DATA)((AUTOWORD)hModule + ImpDir->FirstThunk);
		PIMAGE_THUNK_DATA _INT = (PIMAGE_THUNK_DATA)((AUTOWORD)hModule + ImpDir->OriginalFirstThunk);

		std::unique_ptr<std::stringstream> ImpDirMsg(new std::stringstream);
		for (int i = 0;; i++)
		{
			if (_INT[i].u1.Ordinal == 0)break;
			if (IMAGE_SNAP_BY_ORDINAL(_INT[i].u1.Ordinal))
			{
				*ImpDirMsg << "Ordinal: " << IMAGE_ORDINAL(_INT[i].u1.Ordinal) << std::endl;
			}
			else
			{
				IBN = (PIMAGE_IMPORT_BY_NAME)((AUTOWORD)(hModule)+_INT[i].u1.AddressOfData);
				*ImpDirMsg << "FunctionName: " << (const char*)&(IBN->Name) << std::endl;
			}

			*ImpDirMsg << (LPVOID)_IAT[i].u1.AddressOfData << std::endl;
		}

		return ImpDirMsg;
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
			*ExpDirMsg << "Address: " << (LPVOID)(FunctionDir[OrdinalDir[i]] + (DWORD64)hModule) << " " << (LPCSTR)((DWORD64)hModule + NameDir[i]) << std::endl;

		return ExpDirMsg;
	}

	//------------------------------------------------------------------------------------------------------------------------
	inline bool DebugRegister::GetDr6Bits(type local) const
	{
		return static_cast<bool>(Dr6 & local);
	}

	void DebugRegister::SetDr7Bits(const type local, unsigned char bits)
	{
		bits &= 0b11;
		Dr7 &= ~(0b11 << local);
		Dr7 |= (bits << local);
	}

	DebugRegister::operator CONTEXT() const
	{
		CONTEXT temp;
		temp.ContextFlags = CONTEXT_DEBUG_REGISTERS;
		temp.Dr0 = this->Dr0;
		temp.Dr1 = this->Dr1;
		temp.Dr2 = this->Dr2;
		temp.Dr3 = this->Dr3;
		temp.Dr6 = this->Dr6;
		temp.Dr7 = this->Dr7;
		return temp;
	}

	DebugRegister::DebugRegister(const CONTEXT& context)
	{
		this->Dr0 = context.Dr0;
		this->Dr1 = context.Dr1;
		this->Dr2 = context.Dr2;
		this->Dr3 = context.Dr3;
		this->Dr6 = context.Dr6;
		this->Dr7 = context.Dr7;
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------
	std::map<LPVOID, PVECTORED_EXCEPTION_HANDLER> INT3Hook::AddressToVEH;
	std::mutex INT3Hook::mtx;
	LPVOID INT3Hook::HandleVEH;

	std::thread INT3Hook::INT3HookStartThread([]
		{
			std::cout << "INT3HookStartThread start!!! " << std::endl;
			INT3Hook::HandleVEH = AddVectoredExceptionHandler(1, [](_EXCEPTION_POINTERS* ExceptionInfo)
				{
					std::lock_guard<std::mutex> lock(INT3Hook::mtx);

					if (INT3Hook::AddressToVEH.find(ExceptionInfo->ExceptionRecord->ExceptionAddress) != INT3Hook::AddressToVEH.end())
					{
						if (ExceptionInfo->ExceptionRecord->ExceptionCode == EXCEPTION_BREAKPOINT)
						{
							return INT3Hook::AddressToVEH[ExceptionInfo->ExceptionRecord->ExceptionAddress](ExceptionInfo);
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
			std::lock_guard lock(mtx);
			AddressToVEH[Address] = backcall;
		}
	}
	INT3Hook::~INT3Hook()
	{
		if (this->Address == nullptr) return;

		std::lock_guard<std::mutex> lock(INT3Hook::mtx);
		UnHook();
		INT3Hook::AddressToVEH.erase(this->Address);
	}
	BOOL INT3Hook::Hook(LPVOID Address, PVECTORED_EXCEPTION_HANDLER backcall)
	{
		if (!Address && !this->Address) return false;
		else if (Address) this->Address = (LPBYTE)Address;

		std::unique_lock lock(mtx, std::try_to_lock);

		if (backcall) AddressToVEH[this->Address] = backcall;
		if (AddressToVEH.find(this->Address) == AddressToVEH.end()) return false;

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

	namespace RegisterHook
	{
		LPVOID HandleVEH = nullptr;
		DebugRegister global_context;
		std::mutex mtx;
		std::map<DWORD, DebugRegister> ThrIdToRegister;

		std::thread RegisterHookStartThread([]
			{
				std::cout << "RegisterHookStartThread start!!! " << std::endl;

				HandleVEH = AddVectoredExceptionHandler(1, [](_EXCEPTION_POINTERS* ExceptionInfo)
					{
						AUTOWORD local = ExceptionInfo->ContextRecord->Dr6 & (DebugRegister::B0 | DebugRegister::B1 | DebugRegister::B2 | DebugRegister::B3 | DebugRegister::BD);
						std::unique_lock lock(mtx);
						if (local)
						{
							if (ThrIdToRegister.find(GetCurrentThreadId()) != ThrIdToRegister.end())
							{
								switch (local)
								{
								case DebugRegister::B0:
									return ThrIdToRegister[GetCurrentThreadId()].fc[0](ExceptionInfo);
								case DebugRegister::B1:
									return ThrIdToRegister[GetCurrentThreadId()].fc[1](ExceptionInfo);
								case DebugRegister::B2:
									return ThrIdToRegister[GetCurrentThreadId()].fc[2](ExceptionInfo);
								case DebugRegister::B3:
									return ThrIdToRegister[GetCurrentThreadId()].fc[3](ExceptionInfo);
								case DebugRegister::BD:
									return ThrIdToRegister[GetCurrentThreadId()].fc[4](ExceptionInfo);
								default:
									return (LONG)EXCEPTION_CONTINUE_SEARCH;
								}
							}
						}
						return (LONG)EXCEPTION_CONTINUE_SEARCH;
					});
				AddAllThreadDebug();
			});

		BOOL AddAllThreadDebug()
		{
			THREADENTRY32 t32{ sizeof(THREADENTRY32) };
			CONTEXT tempContext = global_context;
			DWORD processId = GetCurrentProcessId();
			AutoHandle hthreads = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
			if (!hthreads) return FALSE;

			std::unique_lock lock(mtx,std::try_to_lock);

			for (BOOL temp = Thread32First(hthreads, &t32); temp; temp = Thread32Next(hthreads, &t32))
			{
				if (processId != t32.th32OwnerProcessID) continue;
				AutoHandle th = OpenThread(THREAD_ALL_ACCESS, FALSE, t32.th32ThreadID);
				if (!SetThreadContext(th, &tempContext))continue;
				ThrIdToRegister[t32.th32ThreadID] = global_context;
				std::cout << "HookThread: " << t32.th32ThreadID << std::endl;
			}
			return TRUE;
		}

		BOOL AddThreadDebug(DWORD threadId)
		{
			CONTEXT tempContext;
			AutoHandle th = ::OpenThread(THREAD_ALL_ACCESS, FALSE, threadId);
			if (!th)return FALSE;
			std::unique_lock lock(mtx, std::try_to_lock);

			if (ThrIdToRegister.find(threadId) == ThrIdToRegister.end())
			{
				ThrIdToRegister[threadId] = global_context;
				tempContext = global_context;
				return ::SetThreadContext(th, &tempContext);
			}
			else
			{
				tempContext = ThrIdToRegister[threadId];
				return ::SetThreadContext(th, &tempContext);
			}
		}
	}
}