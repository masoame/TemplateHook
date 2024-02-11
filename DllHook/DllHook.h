#pragma once
#include"common.h"

namespace DllHook
{
#ifndef _WIN64
	std::unique_ptr<DWORD32[]> GetFuncArgs(CONTEXT* ct, DWORD32 argc);
#else
	std::unique_ptr<DWORD64[]> GetFuncArgs(CONTEXT* ct, DWORD64 argc);
#endif
	//std::unique_ptr<std::string> GetImportTable(HMODULE hm);
	std::unique_ptr<std::stringstream> GetExportDirectory(HMODULE hModule);

	//INT3断点
	struct INT3Hook
	{
		static std::thread INT3HookStartThread;
		static std::mutex tb_m;
		static std::map<LPVOID, PVECTORED_EXCEPTION_HANDLER> tb;
		static LPVOID HandleVEH;
		LPBYTE Address;
		BYTE Original;

		INT3Hook(LPVOID Address, PVECTORED_EXCEPTION_HANDLER backcall = nullptr);
		virtual ~INT3Hook();

		BOOL Hook(LPVOID Address = nullptr, PVECTORED_EXCEPTION_HANDLER backcall = nullptr);
		BOOL UnHook()const;
	};

	//寄存器断点
	struct RegisterHook
	{
		//寄存器上下文只用drx寄存器用于显示哪些已经打了软断点
		static CONTEXT debug;
		
		RegisterHook();
		~RegisterHook();
		BOOL Hook(LPVOID* Address);
		BOOL UnHook();
	};
}