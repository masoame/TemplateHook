#pragma once
#include"common.hpp"

//���ж�
constexpr BYTE INT3 = 0xCC;
//Ӳ���ж�
//��
constexpr BYTE NOP = 0x90;

namespace DebugHook
{

	std::unique_ptr<char[]> LoadFile(const wchar_t* filename);
	size_t RVAtoFOA(LPVOID pFileBuffer, size_t RVA);


	//������Ӧ�ľ��
	std::unique_ptr<PROCESSENTRY32> SearchProcess(const wchar_t* ProcessName);
	std::unique_ptr<MODULEENTRY32> SearchModule(DWORD th32ProcessID, const wchar_t* ModuleName);

	//����ֻ��Ҫ�������о��Ҳ������ͬEnumProcess��EnumProcessModules
	std::unique_ptr<std::vector<PROCESSENTRY32>> ProcessInfo();
	std::unique_ptr<std::vector<MODULEENTRY32>> ModuleInfo(DWORD th32ProcessID = GetCurrentProcessId());

	//������
	std::unique_ptr<std::stringstream> ShowExportTable(HANDLE ProcessHandle, HMODULE BaseAddress);
	//�����
	std::unique_ptr<std::stringstream> ShowImportTable(HANDLE ProcessHandle, HMODULE BaseAddress);

	//��ȡ������ַ
	FARPROC GetProcAddressEx(HANDLE ProcessHandle, HMODULE BaseAddress, const char* fcName);
	//ע��dll(dll·����inject_dll.exe��ͬһ��)
	DWORD InjectDll(DWORD th32ProcessID, const wchar_t* dll);

	//��ȡ��Ӧ����
	BOOL HookPrc(DWORD th32ProcessID, HMODULE hModule, const char* HookFunction);
	//������������
	BOOL OnExceptionDebugEvent(LPCREATE_PROCESS_DEBUG_INFO pinfo, LPDEBUG_EVENT pde, FARPROC lpfc, BYTE* code);
}
