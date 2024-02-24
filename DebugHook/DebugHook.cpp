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

		if (RVA < pSectionHeader->VirtualAddress)//�ж�RVA�Ƿ���PEͷ��
		{
			if (RVA < pSectionHeader->PointerToRawData)//��ʱRVA==FOA�ж�FOA�᲻�����
				return RVA;
			else
				return NULL;
		}
		for (int i = 0; i < pFileHeader->NumberOfSections; i++)//ѭ�������ڱ�ͷ
		{
			if (RVA >= pSectionHeader[i].VirtualAddress)//�Ƿ��������ڱ��RVA
			{
				if (RVA <= pSectionHeader[i].VirtualAddress + pSectionHeader[i].SizeOfRawData)//�ж��Ƿ����������
					return (RVA - pSectionHeader[i].VirtualAddress) + pSectionHeader[i].PointerToRawData;//ȷ�������󣬼���FOA
			}
			else
				return NULL;
		}
		return NULL;
	}
	//�������н�����Ϣ
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

	//���ؽ��̵�ģ����Ϣ
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
	//������Ӧ���̲�������Ӧ��Ϣ
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
	//������Ӧ���̵�ģ�鲢������Ӧ��Ϣ
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
		//RVA��С4�ֽ�
		int32_t Prc_RVA = 0;
		//��Ŵ�С2�ֽ�
		int16_t Ordinals = 0;

		//��ȡDOSͷ
		if (!ReadProcessMemory(ProcessHandle, BaseAddress, &DosHeader, sizeof(DosHeader), 0) && (DosHeader.e_magic != 0x5A4D)) return FALSE;

		//��ȡNTͷ
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+DosHeader.e_lfanew), &NtHeader, sizeof(NtHeader), 0))return FALSE;
		if (NtHeader.Signature != 0x4550) return FALSE;

		//��ȡ������
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+NtHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), &ExpDir, sizeof(ExpDir), 0))return FALSE;
		//�洢�������ֵ�RVA��
		int32_t* Namestable = new int32_t[ExpDir.NumberOfNames];
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfNames), Namestable, ExpDir.NumberOfNames * sizeof(int32_t), 0))return FALSE;

		for (int i = 0; i != ExpDir.NumberOfNames; i++)
		{
			//ͨ�����ȡ��������
			if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+Namestable[i]), ProcName, sizeof(ProcName), 0)) return FALSE;
			//��ȡ������
			if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfNameOrdinals + sizeof(int16_t) * i), &Ordinals, sizeof(int16_t), 0))return FALSE;
			//��ȡ��ĵ�ַ
			if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfFunctions + Ordinals * sizeof(int32_t)), &Prc_RVA, sizeof(int32_t), 0))return FALSE;
			std::cout << "��ַ:" << (void*)((long long)BaseAddress + Prc_RVA) << "\t���:" << Ordinals << "\t������:" << ProcName << std::endl;
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
		//RVA��С4�ֽ�
		int32_t Prc_RVA = 0;
		//��Ŵ�С2�ֽ�
		int16_t Ordinals = 0;

		//��ȡDOSͷ
		if (!ReadProcessMemory(ProcessHandle, BaseAddress, &DosHeader, sizeof(DosHeader), 0) && (DosHeader.e_magic != 0x5A4D)) return FALSE;
		//��ȡNTͷ
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

	//ע��Զ���߳�
	DWORD InjectDll(DWORD th32ProcessID, const wchar_t* dll)
	{
		//ͨ����Ӧ����id�򿪶�Ӧ����
		AutoHandle<> ph = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, th32ProcessID);
		if (ph)std::cout << "�ɹ��򿪶�Ӧ����" << std::endl;
		else return -1;

		//�ļ�·��
		std::wstring location = std::filesystem::path(os::GetModuleFileNameW().value()).replace_filename(dll);

		//Ϊ��Ӧ�ڴ�������̣����������ڴ�ĵ�ַ
		LPVOID ptrmem = VirtualAllocEx(ph, nullptr, (location.size() + 1) * sizeof(wchar_t), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if (ptrmem)std::cout << "��Ӧ���������ڴ�ɹ�" << std::endl;
		else return -1;

		//д���Ӧ�ڴ�dll·��
		auto temp = WriteProcessMemory(ph, ptrmem, location.data(), (location.size() + 1) * sizeof(wchar_t), 0);
		if (temp)std::cout << "д���Ӧ�ڴ�ɹ�" << std::endl;
		else return -1;

		//ע���߳�
		AutoHandle<> target_thread = ::CreateRemoteThread(ph, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibraryW, ptrmem, 0, 0);
		if (target_thread == false)return -1;

		temp = ::WaitForSingleObject(target_thread, INFINITE);
		if (temp != -1)std::cout << "ע���߳�ִ�����" << std::endl;
		else return -1;

		//�ͷ�������ڴ�
		temp = ::VirtualFreeEx(ph, ptrmem, 0, MEM_RELEASE);
		if (temp)std::cout << "�ڴ��ͷųɹ�" << std::endl;
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
		//RVA��С4�ֽ�
		int32_t Prc_RVA = 0;
		//��Ŵ�С2�ֽ�
		int16_t Ordinals = 0;

		//��ȡDOSͷ
		if (!ReadProcessMemory(ProcessHandle, BaseAddress, &DosHeader, sizeof(DosHeader), 0) && (DosHeader.e_magic != 0x5A4D)) return nullptr;

		//��ȡNTͷ
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+DosHeader.e_lfanew), &NtHeader, sizeof(NtHeader), 0))return nullptr;
		if (NtHeader.Signature != 0x4550) return nullptr;

		//��ȡ������
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+NtHeader.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress), &ExpDir, sizeof(ExpDir), 0))return nullptr;
		//�洢�������ֵ�RVA��
		int32_t* Namestable = new int32_t[ExpDir.NumberOfNames];
		if (!ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfNames), Namestable, ExpDir.NumberOfNames * sizeof(int32_t), 0))return nullptr;

		for (int i = 0; i != ExpDir.NumberOfNames; i++)
		{
			//ͨ�����ȡ��������
			if (ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+Namestable[i]), ProcName, sizeof(ProcName), 0))
				//�ж������Ƿ���ͬ
				if (!strcmp(ProcName, fcName))
					//ͨ������һһ��Ӧ����ű��ȡ�������
					if (ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfNameOrdinals + sizeof(int16_t) * i), &Ordinals, sizeof(int16_t), 0))
						//ͨ��������Ŷ�ȡ������ַRVA
						if (ReadProcessMemory(ProcessHandle, (LPVOID)((SIZE_T)(BaseAddress)+ExpDir.AddressOfFunctions + Ordinals * sizeof(int32_t)), &Prc_RVA, sizeof(int32_t), 0))
							//����ģ��+����RVA��������ַ
							return (FARPROC)((SIZE_T)BaseAddress + Prc_RVA);
		}
		return nullptr;
	}

	BOOL HookPrc(DWORD th32ProcessID, HMODULE hModule, const char* HookFunction)
	{
		//DEBUG��Ӧ����
		if (!DebugActiveProcess(th32ProcessID))return FALSE;

		FARPROC lpfc = 0;
		BYTE code = 0;
		DEBUG_EVENT de;
		DWORD dcstatus;
		//��һ��WaitForDebugEvent�����᷵��CREATE_PROCESS_DEBUG_EVENT��Ϊ���������е�һ����Ҫ����
		CREATE_PROCESS_DEBUG_INFO pinfo;
		while (WaitForDebugEvent(&de, INFINITE))
		{
			dcstatus = DBG_CONTINUE;
			if (de.dwDebugEventCode == CREATE_PROCESS_DEBUG_EVENT)
			{
				pinfo = de.u.CreateProcessInfo;
				//��ȡ��Ӧ����ģ�麯���ĵ�ַ
				lpfc = GetProcAddressEx(pinfo.hProcess, hModule, HookFunction);
				//����������ַ�ǵ�һ���ֽڵ�ֵ
				if (!ReadProcessMemory(pinfo.hProcess, (LPVOID)lpfc, &code, sizeof(BYTE), NULL))return FALSE;
				//����ת��ȥ������һ���ֽڵ��滻ΪINT3���ж�����
				if (!WriteProcessMemory(pinfo.hProcess, (LPVOID)lpfc, &INT3, sizeof(BYTE), NULL))return FALSE;
			}
			else if (de.dwDebugEventCode == EXCEPTION_DEBUG_EVENT)
			{
				//���յ��ж�����
				if (OnExceptionDebugEvent(&pinfo, &de, lpfc, &code))continue;
			}
			else if (de.dwDebugEventCode == EXIT_PROCESS_DEBUG_EVENT)
			{
				std::cout << "DEBUG�����Ѿ��˳�" << std::endl;
				break;
			}
			//����debug
			ContinueDebugEvent(de.dwProcessId, de.dwThreadId, dcstatus);
		}
		Sleep(1);
	}

	//���յ��ж��źź�
	BOOL OnExceptionDebugEvent(LPCREATE_PROCESS_DEBUG_INFO pinfo, LPDEBUG_EVENT pde, FARPROC lpfc, BYTE* code)
	{
		//�������ļĴ���������ؽṹ��
		CONTEXT ctx;
		auto& e = pde->u.Exception.ExceptionRecord;

		if (e.ExceptionCode == EXCEPTION_BREAKPOINT)
		{
			if (e.ExceptionAddress == lpfc)
			{
				//��ԭԭ��������
				if (!WriteProcessMemory(pinfo->hProcess, (LPVOID)lpfc, code, sizeof(BYTE), 0))return FALSE;

				//��ȡ�߳�������
				ctx.ContextFlags = CONTEXT_CONTROL;
				if (!GetThreadContext(pinfo->hThread, &ctx))return FALSE;

				thread_local int num = 0;
				std::cout << "���ú���" << ++num << "��" << std::endl;

				//RIP�Ĵ�������ָ���Ӧ�ĺ�����ַ
#ifdef _WIN64
				ctx.Rip = (DWORD64)lpfc;
#else
				ctx.Eip = (DWORD64)lpfc;
#endif
				if (!SetThreadContext(pinfo->hThread, &ctx))return FALSE;

				//���������߳�
				if (!ContinueDebugEvent(pde->dwProcessId, pde->dwThreadId, DBG_CONTINUE))return FALSE;
				Sleep(1);
				//����ϵ�
				if (!WriteProcessMemory(pinfo->hProcess, (LPVOID)lpfc, &INT3, sizeof(BYTE), NULL))return FALSE;
				return TRUE;
			}
		}
		return FALSE;
	}
}