#pragma once
#include"defs.h"
#include<fstream>
#include<iostream>
#include<filesystem>
#include<map>
#include<thread>
#include<cassert>
#include<regex>
#include<mutex>
#include<sstream>

#include<WinSock2.h>
#include<windows.h>
#include<tlhelp32.h>
#include<psapi.h>

#pragma comment(lib, "Ws2_32.lib")

namespace os
{
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
	AutoHandle(HANDLE h) : h(h) {}
	operator HANDLE() { return h.get(); }
	PHANDLE operator&() { static_assert(sizeof(*this) == sizeof(HANDLE)); assert(h); return (PHANDLE)this; }
	operator bool() { return h.get() != NULL && h.get() != INVALID_HANDLE_VALUE; }
private:
	struct HandleCleaner { void operator()(void* h) { if (h != INVALID_HANDLE_VALUE) HandleCloser()(PermissivePointer{ h }); } };
	std::unique_ptr<void, HandleCleaner> h;
};


template<class KEY, class T>
class ThreadSafeMap : public std::map<KEY, T>
{
	std::mutex mtx;
public:

	auto& operator[](const auto& _Keyval) {
		return __super::operator[](_Keyval);
	}
	auto& operator[](auto&& _Keyval) { // find element matching _Keyval or insert value-initialized value
		return __super::operator[](_Keyval);
	}

	ThreadSafeMap(std::initializer_list<std::pair<const KEY,T>> _Ilist) :std::map<KEY, T>(_Ilist){}

	auto erase(const KEY& _Keyval) noexcept{
		std::unique_lock lock(mtx);
		return std::map<KEY, T>::erase(_Keyval);
	}



};

