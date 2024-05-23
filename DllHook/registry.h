#pragma once
#include"common.hpp"
//����ע���
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

	//���淵����Ϣ
	void CommonMessageBack(SOCKET* tcp);
}