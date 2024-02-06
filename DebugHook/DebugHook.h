#pragma once
#include"common.h"

//软中断
constexpr BYTE INT3 = 0xCC;
//硬件中断
//空
constexpr BYTE NOP = 0x90;

namespace DebugHook
{
	//搜索对应的句柄
	std::unique_ptr<PROCESSENTRY32> SearchProcess(const wchar_t* ProcessName);
	std::unique_ptr<MODULEENTRY32> SearchModule(DWORD th32ProcessID, const wchar_t* ModuleName);

	//单纯只是要遍历所有句柄也可以是同EnumProcess或EnumProcessModules
	std::unique_ptr<std::vector<PROCESSENTRY32>> ProcessInfo();
	std::unique_ptr<std::vector<MODULEENTRY32>> ModuleInfo(DWORD th32ProcessID);

	//导出表
	BOOL ShowExportTable(HANDLE ProcessHandle, HMODULE BaseAddress);
	//导入表
	BOOL ShowImportTable(HANDLE ProcessHandle, HMODULE BaseAddress);

	//获取函数地址
	FARPROC GetProcAddressEx(HANDLE ProcessHandle, HMODULE BaseAddress, const char* fcName);
	//注入dll(dll路径与inject_dll.exe在同一级)
	DWORD InjectDll(DWORD th32ProcessID, const wchar_t* dll);

	//勾取相应函数
	BOOL HookPrc(DWORD th32ProcessID, HMODULE hModule, const char* HookFunction);
	//当函数被调用
	BOOL OnExceptionDebugEvent(LPCREATE_PROCESS_DEBUG_INFO pinfo, LPDEBUG_EVENT pde, FARPROC lpfc, BYTE* code);
}
