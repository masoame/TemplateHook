#pragma once
#include"common.h"

namespace DllHook
{
#ifndef _WIN64
	using AUTOWORD = DWORD32;
	std::unique_ptr<DWORD32[]> GetFuncArgs(const CONTEXT* ct, const DWORD32 argc);
#else
	using AUTOWORD = DWORD64;
	std::unique_ptr<DWORD64[]> GetFuncArgs(const CONTEXT* ct, const DWORD64 argc);
#endif

	struct DebugAddressRegister
	{
		AUTOWORD Dr0;
		AUTOWORD Dr1;
		AUTOWORD Dr2;
		AUTOWORD Dr3;
		struct __Dr6
		{
			AUTOWORD dr6;
			enum type :AUTOWORD
			{
				B0 = 1, B1 = 1 << 1, B2 = 1 << 2, B3 = 1 << 3, BD = 1 << 13, BS = 1 << 14, BT = 1 << 15
			};
			operator AUTOWORD& () { return this->dr6; }
			void operator=(const AUTOWORD& dr6) { this->dr6 = dr6; }
			bool GetBits(type local) const;
		}Dr6;
		struct __Dr7
		{
			AUTOWORD dr7;
			enum type :unsigned char
			{
				LG0 = 0, LG1 = 2, LG2 = 4, LG3 = 6,
				LEGE = 8, GD = 13,
				LEN0 = 18, LEN1 = 22, LEN2 = 26, LEN3 = 30,
				RW0 = 16, RW1 = 20, RW2 = 24, RW3 = 28
			};
			operator AUTOWORD& () { return this->dr7; }
			void operator=(const AUTOWORD& dr7) { this->dr7 = dr7; }
			void SetBits(const type local, unsigned char bits);
		}Dr7;
		void operator=(const CONTEXT& context);
	};

	std::unique_ptr<std::stringstream> GetImportDirectory(const HMODULE hModule);
	std::unique_ptr<std::stringstream> GetExportDirectory(const HMODULE hModule);

	//INT3¶Ïµã
	struct INT3Hook
	{
		static std::thread INT3HookStartThread;
		static std::mutex tb_m;
		static std::map<LPVOID, PVECTORED_EXCEPTION_HANDLER> tb;
		static LPVOID HandleVEH;

		LPBYTE Address;
		BYTE Original;

		INT3Hook(const LPVOID Address, const PVECTORED_EXCEPTION_HANDLER backcall = nullptr);
		virtual ~INT3Hook();

		BOOL Hook(const LPVOID Address = nullptr, const PVECTORED_EXCEPTION_HANDLER backcall = nullptr);
		BOOL UnHook()const;
	};

	//¼Ä´æÆ÷¶Ïµã
	struct RegisterHook
	{
		static std::thread RegisterHookStartThread;
		static LPVOID HandleVEH;
		static DebugAddressRegister global_context;

		static std::mutex tb_m;
		static std::map<DWORD, RegisterHook> tb;
		DWORD threadid;
		DebugAddressRegister local_context;

		RegisterHook();
		~RegisterHook();
		BOOL Hook(LPVOID* Address);
		BOOL UnHook();
	};
}