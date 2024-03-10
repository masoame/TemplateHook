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

	struct DebugRegister
	{
		AUTOWORD Dr0;
		AUTOWORD Dr1;
		AUTOWORD Dr2;
		AUTOWORD Dr3;
		enum type :AUTOWORD
		{
			LG0 = 0, LG1 = 2, LG2 = 4, LG3 = 6,
			LEGE = 8, GD = 13,
			LEN0 = 18, LEN1 = 22, LEN2 = 26, LEN3 = 30,
			RW0 = 16, RW1 = 20, RW2 = 24, RW3 = 28,
			//--------------------------------------------------------------------------------------------
			B0 = 1, B1 = 1 << 1, B2 = 1 << 2, B3 = 1 << 3, BD = 1 << 13, BS = 1 << 14, BT = 1 << 15
		};
		struct
		{
			AUTOWORD dr6;
			operator AUTOWORD () const { return this->dr6; }
			operator AUTOWORD& () { return this->dr6; }
			void operator=(const AUTOWORD& dr6) { this->dr6 = dr6; }
		}Dr6;
		struct
		{
			AUTOWORD dr7;
			operator AUTOWORD () const { return this->dr7; }
			operator AUTOWORD& () { return this->dr7; }
			void operator=(const AUTOWORD& dr7) { this->dr7 = dr7; }
		}Dr7;

		operator CONTEXT() const;
		PVECTORED_EXCEPTION_HANDLER fc[5];

		DebugRegister() :Dr0(0), Dr1(0), Dr2(0), Dr3(0), Dr6{ 0 }, Dr7{ 0 } {};
		DebugRegister(const CONTEXT& context) :Dr0(context.Dr0), Dr1(context.Dr1), Dr2(context.Dr2), Dr3(context.Dr3), Dr6{ context.Dr6 }, Dr7{ context.Dr7 } {};

		bool GetDr6Bits(type local) const;
		void SetDr7Bits(const type local, unsigned char bits);
	};
	std::unique_ptr<std::stringstream> GetImportDirectory(const HMODULE hModule);
	std::unique_ptr<std::stringstream> GetExportDirectory(const HMODULE hModule);

	//INT3¶Ïµã
	struct INT3Hook
	{
		static std::thread INT3HookStartThread;
		static std::mutex mtx;
		static std::map<LPVOID, PVECTORED_EXCEPTION_HANDLER> AddressToVEH;
		static LPVOID HandleVEH;

		LPBYTE Address;
		BYTE Original;
		INT3Hook(LPVOID Address, const PVECTORED_EXCEPTION_HANDLER backcall = nullptr);
		virtual ~INT3Hook();

		BOOL Hook(LPVOID Address = nullptr, const PVECTORED_EXCEPTION_HANDLER backcall = nullptr);
		BOOL UnHook()const;
	};

	//¼Ä´æÆ÷¶Ïµã
	namespace RegisterHook
	{
		extern std::thread RegisterHookStartThread;
		extern LPVOID HandleVEH;
		extern DebugRegister global_context;
		extern std::mutex mtx;
		extern std::map<DWORD, DebugRegister> ThrIdToRegister;

		extern void Flush_GlobalDebug() noexcept;
		extern BOOL Insert_ThreadDebug(DWORD threadId);
	};
}