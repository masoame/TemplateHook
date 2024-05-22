#pragma once
#include"defs.h"
#include<iostream>
#include<filesystem>
#include<fstream>
#include<thread>
#include<cassert>
#include<regex>
#include<optional>
#include<memory>
#include<sstream>
#include<mutex>
#include<queue>

#include<windows.h>
#include<tlhelp32.h>
#include<Psapi.h>
#include<winternl.h>


//自定义命名空间
namespace os
{
	//获取系统模块路径
	inline std::optional<std::wstring> GetModuleFileNameW(HMODULE module = NULL)
	{
		std::vector<wchar_t> buffer(MAX_PATH);
		if (GetModuleFileNameW(module, buffer.data(), MAX_PATH)) return buffer.data();
		return {};
		
	}
}

struct PermissivePointer
{
	template <typename T>
	operator T* () { return (T*)p; }
	void* p;
};

template <auto F>
using Functor = std::integral_constant<std::remove_reference_t<decltype(F)>, F>;

template<typename HandleCloser = Functor<CloseHandle>>
class AutoHandle
{
public:
	AutoHandle() : h(nullptr) {}
	AutoHandle(HANDLE h) : h(h) {}
	void operator=(HANDLE h) { this->h.reset(h); }
	operator HANDLE() { return h.get(); }
	PHANDLE operator&() { static_assert(sizeof(*this) == sizeof(HANDLE)); assert(h); return (PHANDLE)this; }
	operator bool() { return (h.get() != NULL) && (h.get() != INVALID_HANDLE_VALUE); }
private:
	struct HandleCleaner { void operator()(void* h) { if (h != INVALID_HANDLE_VALUE)HandleCloser()(PermissivePointer{ h }); } };
	std::unique_ptr<void, HandleCleaner> h;
};
