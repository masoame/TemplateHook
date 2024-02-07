#pragma once
#include"common.h"
using PVEH = PVECTORED_EXCEPTION_HANDLER;

namespace DllHook
{
#ifndef _WIN64
	std::unique_ptr<DWORD32[]> GetFuncArgs(CONTEXT* ct, DWORD32 argc);
#else
	std::unique_ptr<DWORD64[]> GetFuncArgs(CONTEXT* ct,DWORD64 argc);
#endif
	//std::unique_ptr<std::string> GetImportTable(HMODULE hm);
	std::unique_ptr<std::stringstream> GetExportDirectory(HMODULE hModule);



	//INT3断点
	struct INT3Hook
	{
		static std::thread INT3HookStartThread;
		static std::mutex tb_m;
		static std::map<LPVOID, PVEH> tb;
		static LPVOID HandleVEH;

		//保存断点地址
		LPBYTE Address;
		BYTE Original;

		INT3Hook(LPVOID Address, PVEH backcall = nullptr);
		virtual ~INT3Hook();
		//加钩子
		BOOL Hook(LPVOID Address = nullptr, PVEH backcall = nullptr);
		//脱钩
		BOOL UnHook()const;
	};

	//寄存器断点
	struct RegisterHook
	{
		//寄存器上下文只用drx寄存器用于显示哪些已经打了软断点
		static CONTEXT debug;

		//加钩子
		BOOL Hook(LPVOID* Address);

		//脱钩
		BOOL UnHook();
	};
}