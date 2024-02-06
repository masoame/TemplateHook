#pragma once

#include"defs.h"
#include<iostream>
#include<filesystem>
#include<optional>
#include<tuple>
#include<map>
#include<set>
#include<thread>
#include<cassert>
#include<regex>
#include<mutex>

#include<WinSock2.h>
#include<windows.h>
#include<tlhelp32.h>
#include<Psapi.h>
#pragma comment(lib, "Ws2_32.lib")

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

//强转为对应指针
struct PermissivePointer
{
	template <typename T>
	operator T* () { return (T*)p; }
	void* p;
};

//提取函数地址固定为类
template <auto F>
using Functor = std::integral_constant<std::remove_reference_t<decltype(F)>, F>;

//管理句柄用
template<typename HandleCloser = Functor<CloseHandle>>
class AutoHandle
{
public:
	//默认构造函数
	AutoHandle(HANDLE h) : h(h) {}
	//类型转换为句柄直接返回指针的值
	operator HANDLE() { return h.get(); }
	//取地址时直接返回指针类的地址(检查为空指针直接报错,MSVC智能指针与void*大小相同其他平台未测试)
	PHANDLE operator&() { static_assert(sizeof(*this) == sizeof(HANDLE)); assert(h); return (PHANDLE)this; }
	//类型转换为bool值
	operator bool() { return h.get() != NULL && h.get() != INVALID_HANDLE_VALUE; }
private:
	//自定义覆盖default_delete
	struct HandleCleaner { void operator()(void* h) { if (h != INVALID_HANDLE_VALUE) HandleCloser()(PermissivePointer{ h }); } };
	//智能指针管理句柄的释放
	std::unique_ptr<void, HandleCleaner> h;
};