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

//�Զ��������ռ�
namespace os
{
	//��ȡϵͳģ��·��
	inline std::optional<std::wstring> GetModuleFileNameW(HMODULE module = NULL)
	{
		std::vector<wchar_t> buffer(MAX_PATH);
		if (GetModuleFileNameW(module, buffer.data(), MAX_PATH)) return buffer.data();
		return {};
	}
}

//ǿתΪ��Ӧָ��
struct PermissivePointer
{
	template <typename T>
	operator T* () { return (T*)p; }
	void* p;
};

//��ȡ������ַ�̶�Ϊ��
template <auto F>
using Functor = std::integral_constant<std::remove_reference_t<decltype(F)>, F>;

//��������
template<typename HandleCloser = Functor<CloseHandle>>
class AutoHandle
{
public:
	//Ĭ�Ϲ��캯��
	AutoHandle(HANDLE h) : h(h) {}
	//����ת��Ϊ���ֱ�ӷ���ָ���ֵ
	operator HANDLE() { return h.get(); }
	//ȡ��ַʱֱ�ӷ���ָ����ĵ�ַ(���Ϊ��ָ��ֱ�ӱ���,MSVC����ָ����void*��С��ͬ����ƽ̨δ����)
	PHANDLE operator&() { static_assert(sizeof(*this) == sizeof(HANDLE)); assert(h); return (PHANDLE)this; }
	//����ת��Ϊboolֵ
	operator bool() { return h.get() != NULL && h.get() != INVALID_HANDLE_VALUE; }
private:
	//�Զ��帲��default_delete
	struct HandleCleaner { void operator()(void* h) { if (h != INVALID_HANDLE_VALUE) HandleCloser()(PermissivePointer{ h }); } };
	//����ָ����������ͷ�
	std::unique_ptr<void, HandleCleaner> h;
};