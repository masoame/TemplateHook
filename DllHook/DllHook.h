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



	//INT3�ϵ�
	struct INT3Hook
	{
		static std::thread INT3HookStartThread;
		static std::mutex tb_m;
		static std::map<LPVOID, PVEH> tb;
		static LPVOID HandleVEH;

		//����ϵ��ַ
		LPBYTE Address;
		BYTE Original;

		INT3Hook(LPVOID Address, PVEH backcall = nullptr);
		virtual ~INT3Hook();
		//�ӹ���
		BOOL Hook(LPVOID Address = nullptr, PVEH backcall = nullptr);
		//�ѹ�
		BOOL UnHook()const;
	};

	//�Ĵ����ϵ�
	struct RegisterHook
	{
		//�Ĵ���������ֻ��drx�Ĵ���������ʾ��Щ�Ѿ�������ϵ�
		static CONTEXT debug;

		//�ӹ���
		BOOL Hook(LPVOID* Address);

		//�ѹ�
		BOOL UnHook();
	};
}