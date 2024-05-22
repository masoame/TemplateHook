#pragma once
#include"common.hpp"
//操作注册表
namespace registry
{
	constexpr wchar_t keypath[] = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";

	struct InstallMsgFrame
	{
		wchar_t name[MAX_PATH];
		wchar_t version[MAX_PATH];
		wchar_t date[MAX_PATH];
		wchar_t install[MAX_PATH];
		wchar_t unstall[MAX_PATH];
	};

	std::unique_ptr<std::wstringstream> GetSoftwareMsg();

	//常规返回信息
	void CommonMessageBack(SOCKET* tcp);
}