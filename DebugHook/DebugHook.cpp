#include"DebugHook.h"

namespace DebugHook
{
	size_t RVAtoFOA(LPVOID pFileBuffer, size_t RVA)
	{
		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)pFileBuffer;
		if (pDosHeader->e_magic != 0x5A4D) return NULL;
		PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((DWORD64)pFileBuffer + pDosHeader->e_lfanew);
		if (pNtHeader->Signature != 0x4550) return NULL;

		PIMAGE_FILE_HEADER pFileHeader = &pNtHeader->FileHeader;
		PIMAGE_OPTIONAL_HEADER pOptionHeader = &pNtHeader->OptionalHeader;

		PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pNtHeader);

		if (RVA < pSectionHeader->VirtualAddress)//判断RVA是否在PE头区
		{
			if (RVA < pSectionHeader->PointerToRawData)//此时RVA==FOA判断FOA会不会溢出
				return RVA;
			else
				return NULL;
		}
		for (int i = 0; i < pFileHeader->NumberOfSections; i++)//循环遍历节表头
		{
			if (RVA >= pSectionHeader[i].VirtualAddress)//是否大于这个节表的RVA
			{
				if (RVA <= pSectionHeader[i].VirtualAddress + pSectionHeader[i].SizeOfRawData)//判断是否在这个节区
					return (RVA - pSectionHeader[i].VirtualAddress) + pSectionHeader[i].PointerToRawData;//确定节区后，计算FOA
			}
			else
				return NULL;
		}
		return NULL;
	}
	//返回所有进程信息
	std::unique_ptr<std::vector<PROCESSENTRY32>> ProcessInfo()
	{
		std::unique_ptr<std::vector<PROCESSENTRY32>> ret(new std::vector<PROCESSENTRY32>);
		ret->reserve(400);

		PROCESSENTRY32 pe32{};
		pe32.dwSize = sizeof(pe32);
		AutoHandle hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE) return nullptr;

		BOOL bMore = ::Process32First(hProcessSnap, &pe32);

		while (bMore)
		{
			ret->emplace_back(pe32);
			bMore = ::Process32Next(hProcessSnap, &pe32);
		};

		return ret;
	}

	//返回进程的模块信息
	std::unique_ptr<std::vector<MODULEENTRY32>> ModuleInfo(DWORD th32ProcessID)
	{
		std::unique_ptr<std::vector<MODULEENTRY32>> ret(new std::vector<MODULEENTRY32>);
		ret->reserve(20);

		MODULEENTRY32 pe32{};
		pe32.dwSize = sizeof(pe32);
		AutoHandle hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, th32ProcessID);
		if (hProcessSnap == INVALID_HANDLE_VALUE) return nullptr;
		BOOL bMore = ::Module32First(hProcessSnap, &pe32);

		while (bMore)
		{
			ret->emplace_back(pe32);
			bMore = ::Module32Next(hProcessSnap, &pe32);
		};

		return ret;
	}
	//搜索对应进程并返回相应信息
	std::unique_ptr<PROCESSENTRY32> SearchProcess(const wchar_t* ProcessName)
	{
		std::unique_ptr<PROCESSENTRY32> pe32(new PROCESSENTRY32{});
		pe32->dwSize = sizeof(*pe32);
		AutoHandle hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE) return {};

		BOOL bMore = ::Process32First(hProcessSnap, pe32.get());

		while (bMore)
		{
			if (!wcscmp(pe32->szExeFile, ProcessName)) return pe32;
			bMore = ::Process32Next(hProcessSnap, pe32.get());
		};

		return {};
	}
	//搜索对应进程的模块并返回相应信息
	std::unique_ptr<MODULEENTRY32> SearchModule(DWORD th32ProcessID, const wchar_t* ModuleName)
	{
		std::unique_ptr<MODULEENTRY32> pe32(new MODULEENTRY32);

		pe32->dwSize = sizeof(*pe32);
		AutoHandle hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, th32ProcessID);
		if (hProcessSnap == INVALID_HANDLE_VALUE) return nullptr;

		BOOL bMore = ::Module32First(hProcessSnap, pe32.get());

		while (bMore)
		{
			if (!lstrcmpiW(pe32->szModule, ModuleName)) return pe32;
			bMore = ::Module32Next(hProcessSnap, pe32.get());
		};
		return {};
	}

	BOOL ShowExportTable(HANDLE ProcessHandle, HMODULE BaseAddress)
	{
		if (!ProcessHandle || !BaseAddress) return FALSE;
		char ProcName[256];
		IMAGE_DOS_HEADER DosHeader;
		IMAGE_NT_HEADERS NtHeader;
		IMAGE_EXPORT_DIRECTORY ExpDir;
		//RVA大小4字节
		int32_t Prc_RVA = 0;
		//序号大小2字节
		int16_t Ordinals = 0;

		//读取DOS头
		if (!ReadProcessMemory(ProcessHandle, BaseAddress, &DosHeader, sizeof(DosHeader), 0) && (DosHeader.e_magic != 0x5A4D)) return FALSE;

		//读取NT头
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+DosHeader.e_lfanew), &NtHeader, sizeof(NtHeader), 0))return FALSE;
		if (NtHeader.Signature != 0x4550) return FALSE;

		//读取导出表
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+NtHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), &ExpDir, sizeof(ExpDir), 0))return FALSE;
		//存储函数名字的RVA表
		int32_t* Namestable = new int32_t[ExpDir.NumberOfNames];
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfNames), Namestable, ExpDir.NumberOfNames * sizeof(int32_t), 0))return FALSE;

		for (int i = 0; i != ExpDir.NumberOfNames; i++)
		{
			//通过表读取函数名字
			if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+Namestable[i]), ProcName, sizeof(ProcName), 0)) return FALSE;
			//读取表的序号
			if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfNameOrdinals + sizeof(int16_t) * i), &Ordinals, sizeof(int16_t), 0))return FALSE;
			//读取表的地址
			if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfFunctions + Ordinals * sizeof(int32_t)), &Prc_RVA, sizeof(int32_t), 0))return FALSE;
			std::cout << "地址:" << (void*)((long long)BaseAddress + Prc_RVA) << "\t序号:" << Ordinals << "\t函数名:" << ProcName << std::endl;
		}
		return FALSE;
	}

	BOOL ShowImportTable(HANDLE ProcessHandle, HMODULE BaseAddress)
	{
		if (!ProcessHandle || !BaseAddress) return FALSE;
		char Name[256];
		IMAGE_DOS_HEADER DosHeader;
		IMAGE_NT_HEADERS NtHeader;
		IMAGE_IMPORT_DESCRIPTOR ImpDes;
		PIMAGE_IMPORT_BY_NAME IMBN = (PIMAGE_IMPORT_BY_NAME)Name;
		IMAGE_THUNK_DATA _IAT, _INT;
		//RVA大小4字节
		int32_t Prc_RVA = 0;
		//序号大小2字节
		int16_t Ordinals = 0;

		//读取DOS头
		if (!ReadProcessMemory(ProcessHandle, BaseAddress, &DosHeader, sizeof(DosHeader), 0) && (DosHeader.e_magic != 0x5A4D)) return FALSE;
		//读取NT头
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+DosHeader.e_lfanew), &NtHeader, sizeof(NtHeader), 0))return FALSE;
		if (NtHeader.Signature != 0x4550) return FALSE;

		for (int i = 0; ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+NtHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress + sizeof(ImpDes) * i), &ImpDes, sizeof(ImpDes), 0); i++)
		{
			if (ImpDes.Characteristics == 0)break;
			if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ImpDes.Name), &Name, sizeof(Name), 0))return -1;
			std::cout << Name << std::endl;

			for (int j = 0; ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ImpDes.OriginalFirstThunk + sizeof(_INT) * j), &_INT, sizeof(_INT), 0); j++)
			{
				if (_INT.u1.Ordinal == 0)break;
				if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ImpDes.FirstThunk + sizeof(_IAT) * j), &_IAT, sizeof(_IAT), 0))break;
				std::cout << "adress: " << (void*)_IAT.u1.AddressOfData;

				if (IMAGE_SNAP_BY_ORDINAL(_INT.u1.Ordinal))
				{
					std::cout << "Ordinal: " << IMAGE_ORDINAL(_INT.u1.Ordinal) << std::endl;
				}
				else
				{
					if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+_INT.u1.AddressOfData), &Name, sizeof(Name), 0))break;
					std::cout << " Ordinal: " << IMBN->Hint << " \tFunctionName: " << (const char*)&IMBN->Name << std::endl;
				}
			}
		}

		return TRUE;
	}

	//注入远程线程
	DWORD InjectDll(DWORD th32ProcessID, const wchar_t* dll)
	{
		//通过相应进程id打开对应进程
		AutoHandle<> ph = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, th32ProcessID);
		if (ph)std::cout << "成功打开对应进程" << std::endl;
		else return -1;

		//文件路径
		std::wstring location = std::filesystem::path(os::GetModuleFileNameW().value()).replace_filename(dll);

		//为对应内存申请进程，返回申请内存的地址
		LPVOID ptrmem = VirtualAllocEx(ph, nullptr, (location.size() + 1) * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (ptrmem)std::cout << "对应进程申请内存成功" << std::endl;
		else return -1;

		//写入对应内存dll路径
		auto temp = WriteProcessMemory(ph, ptrmem, location.data(), (location.size() + 1) * sizeof(wchar_t), 0);
		if (temp)std::cout << "写入对应内存成功" << std::endl;
		else return -1;

		//注入线程
		AutoHandle<> target_thread = ::CreateRemoteThread(ph, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryW, ptrmem, 0, 0);
		if (target_thread == false)return -1;

		temp = ::WaitForSingleObject(target_thread, INFINITE);
		if (temp != -1)std::cout << "注入线程执行完毕" << std::endl;
		else return -1;

		//释放申请的内存
		temp = ::VirtualFreeEx(ph, ptrmem, 0, MEM_RELEASE);
		if (temp)std::cout << "内存释放成功" << std::endl;
		else return -1;

		return 0;
	}

	FARPROC GetProcAddressEx(HANDLE ProcessHandle, HMODULE BaseAddress, const char* fcName)
	{
		if (!ProcessHandle || !BaseAddress || !fcName) return nullptr;

		char ProcName[260] = { 0 };
		IMAGE_DOS_HEADER DosHeader;
		IMAGE_NT_HEADERS NtHeader;
		IMAGE_EXPORT_DIRECTORY ExpDir;
		//RVA大小4字节
		int32_t Prc_RVA = 0;
		//序号大小2字节
		int16_t Ordinals = 0;

		//读取DOS头
		if (!ReadProcessMemory(ProcessHandle, BaseAddress, &DosHeader, sizeof(DosHeader), 0) && (DosHeader.e_magic != 0x5A4D)) return nullptr;

		//读取NT头
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+DosHeader.e_lfanew), &NtHeader, sizeof(NtHeader), 0))return nullptr;
		if (NtHeader.Signature != 0x4550) return nullptr;

		//读取导出表
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+NtHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), &ExpDir, sizeof(ExpDir), 0))return nullptr;
		//存储函数名字的RVA表
		int32_t* Namestable = new int32_t[ExpDir.NumberOfNames];
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfNames), Namestable, ExpDir.NumberOfNames * sizeof(int32_t), 0))return nullptr;

		for (int i = 0; i != ExpDir.NumberOfNames; i++)
		{
			//通过表读取函数名字
			if (ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+Namestable[i]), ProcName, sizeof(ProcName), 0))
				//判断名字是否相同
				if (!strcmp(ProcName, fcName))
					//通过名字一一对应的序号表读取所在序号
					if (ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfNameOrdinals + sizeof(int16_t) * i), &Ordinals, sizeof(int16_t), 0))
						//通过所在序号读取函数地址RVA
						if (ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfFunctions + Ordinals * sizeof(int32_t)), &Prc_RVA, sizeof(int32_t), 0))
							//返回模块+函数RVA即函数地址
							return (FARPROC)((SIZE_T)BaseAddress + Prc_RVA);
		}
		return nullptr;
	}

	BOOL HookPrc(DWORD th32ProcessID, HMODULE hModule, const char* HookFunction)
	{
		//DEBUG对应进程
		if (!DebugActiveProcess(th32ProcessID))return FALSE;

		FARPROC lpfc = 0;
		BYTE code = 0;
		DEBUG_EVENT de;
		DWORD dcstatus;
		//第一次WaitForDebugEvent函数会返回CREATE_PROCESS_DEBUG_EVENT因为在联合体中第一次需要保存
		CREATE_PROCESS_DEBUG_INFO pinfo;
		while (WaitForDebugEvent(&de, INFINITE))
		{
			dcstatus = DBG_CONTINUE;
			if (de.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
			{
				pinfo = de.u.CreateProcessInfo;
				//获取对应进程模块函数的地址
				lpfc = GetProcAddressEx(pinfo.hProcess, hModule, HookFunction);
				//读出函数地址那第一个字节的值
				if (!ReadProcessMemory(pinfo.hProcess, (LPVOID)lpfc, &code, sizeof(BYTE), NULL))return FALSE;
				//将跳转过去函数第一个字节的替换为INT3即中断请求
				if (!WriteProcessMemory(pinfo.hProcess, (LPVOID)lpfc, &INT3, sizeof(BYTE), NULL))return FALSE;
			}
			else if (de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
			{
				//接收到中断请求
				if (OnExceptionDebugEvent(&pinfo, &de, lpfc, &code))continue;
			}
			else if (de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
			{
				std::cout << "DEBUG进程已经退出" << std::endl;
				break;
			}
			//继续debug
			ContinueDebugEvent(de.dwProcessId, de.dwThreadId, dcstatus);
		}
		Sleep(1);
	}

	//接收到中断信号后
	BOOL OnExceptionDebugEvent(LPCREATE_PROCESS_DEBUG_INFO pinfo, LPDEBUG_EVENT pde, FARPROC lpfc, BYTE* code)
	{
		//处理器的寄存器数据相关结构体
		CONTEXT ctx;
		auto& e = pde->u.Exception.ExceptionRecord;

		if (e.ExceptionCode == EXCEPTION_BREAKPOINT)
		{
			if (e.ExceptionAddress == lpfc)
			{
				//还原原本的内容
				if (!WriteProcessMemory(pinfo->hProcess, (LPVOID)lpfc, code, sizeof(BYTE), 0))return FALSE;

				//获取线程上下文
				ctx.ContextFlags = CONTEXT_CONTROL;
				if (!GetThreadContext(pinfo->hThread, &ctx))return FALSE;

				thread_local int num = 0;
				std::cout << "调用函数" << ++num << "次" << std::endl;

				//RIP寄存器重新指向对应的函数地址
#ifdef _WIN64
				ctx.Rip = (DWORD64)lpfc;
#else
				ctx.Eip = (DWORD64)lpfc;
#endif
				if (!SetThreadContext(pinfo->hThread, &ctx))return FALSE;

				//重新运行线程
				if (!ContinueDebugEvent(pde->dwProcessId, pde->dwThreadId, DBG_CONTINUE))return FALSE;
				Sleep(1);
				//打入断点
				if (!WriteProcessMemory(pinfo->hProcess, (LPVOID)lpfc, &INT3, sizeof(BYTE), NULL))return FALSE;
				return TRUE;
			}
		}
		return FALSE;
	}
}