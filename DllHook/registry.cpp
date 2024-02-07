#include"registry.h"
namespace registry
{
	std::unique_ptr<std::wstringstream> GetInstallMsg()
	{
		HKEY reskey, valkey;
		wchar_t lpName[MAX_PATH]{ 0 };

		DWORD dwIndex = 0;
		DWORD Namelen = MAX_PATH;

		DWORD type = REG_SZ;
		DWORD dwszie;
		InstallMsgFrame imf;
		std::unique_ptr<std::wstringstream> ss(new std::wstringstream);

		if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, (LPCWSTR)&keypath, NULL, KEY_ALL_ACCESS, &reskey) != ERROR_SUCCESS) return FALSE;

		while (RegEnumKeyExW(reskey, dwIndex++, lpName, &Namelen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			if (RegOpenKeyExW(reskey, lpName, NULL, KEY_ALL_ACCESS, &valkey) == ERROR_SUCCESS)
			{
				dwszie = MAX_PATH * 2;
				if (RegQueryValueExW(valkey, L"DisplayName", NULL, &type, (LPBYTE)imf.name, &dwszie) == ERROR_FILE_NOT_FOUND) { imf.name[0] = 0; goto Lab_01; }
				dwszie = MAX_PATH * 2;
				if (RegQueryValueExW(valkey, L"DisplayVersion", NULL, &type, (LPBYTE)imf.version, &dwszie) == ERROR_FILE_NOT_FOUND) { imf.version[0] = 0; }
				dwszie = MAX_PATH * 2;
				if (RegQueryValueExW(valkey, L"InstallDate", NULL, &type, (LPBYTE)imf.date, &dwszie) == ERROR_FILE_NOT_FOUND) { imf.date[0] = 0; }
				dwszie = MAX_PATH * 2;
				if (RegQueryValueExW(valkey, L"InstallLocation", NULL, &type, (LPBYTE)imf.install, &dwszie) == ERROR_FILE_NOT_FOUND) { imf.install[0] = 0; }
				dwszie = MAX_PATH * 2;
				if (RegQueryValueExW(valkey, L"UninstallString", NULL, &type, (LPBYTE)imf.unstall, &dwszie) == ERROR_FILE_NOT_FOUND) { imf.unstall[0] = 0; }
				*ss << L"DisplayName: " << imf.name << std::endl
					<< L"DisplayVersion: " << imf.version << std::endl
					<< L"InstallDate: " << imf.date << std::endl
					<< L"InstallLocation: " << imf.install << std::endl
					<< L"UninstallString: " << imf.unstall << std::endl << std::endl;
			}
			Lab_01:
			Namelen = MAX_PATH;
			RegCloseKey(valkey);
		}
		RegCloseKey(reskey);


		dwIndex = 0;
		if (RegOpenKeyExW(HKEY_CURRENT_USER, (LPCWSTR)&keypath, NULL, KEY_ALL_ACCESS, &reskey) != ERROR_SUCCESS) return FALSE;
		while (RegEnumKeyExW(reskey, dwIndex++, lpName, &Namelen, NULL, NULL, NULL, NULL) == ERROR_SUCCESS)
		{
			if (RegOpenKeyExW(reskey, lpName, NULL, KEY_ALL_ACCESS, &valkey) == ERROR_SUCCESS)
			{
				dwszie = MAX_PATH * 2;
				if (RegQueryValueExW(valkey, L"DisplayName", NULL, &type, (LPBYTE)imf.name, &dwszie) == ERROR_FILE_NOT_FOUND) { imf.name[0] = 0; goto Lab_02; }
				dwszie = MAX_PATH * 2;
				if (RegQueryValueExW(valkey, L"DisplayVersion", NULL, &type, (LPBYTE)imf.version, &dwszie) == ERROR_FILE_NOT_FOUND) { imf.version[0] = 0; }
				dwszie = MAX_PATH * 2;
				if (RegQueryValueExW(valkey, L"InstallDate", NULL, &type, (LPBYTE)imf.date, &dwszie) == ERROR_FILE_NOT_FOUND) { imf.date[0] = 0; }
				dwszie = MAX_PATH * 2;
				if (RegQueryValueExW(valkey, L"InstallLocation", NULL, &type, (LPBYTE)imf.install, &dwszie) == ERROR_FILE_NOT_FOUND) { imf.install[0] = 0; }
				dwszie = MAX_PATH * 2;
				if (RegQueryValueExW(valkey, L"UninstallString", NULL, &type, (LPBYTE)imf.unstall, &dwszie) == ERROR_FILE_NOT_FOUND) { imf.unstall[0] = 0; }
				*ss << L"DisplayName: " << imf.name << std::endl
					<< L"DisplayVersion: " << imf.version << std::endl
					<< L"InstallDate: " << imf.date << std::endl
					<< L"InstallLocation: " << imf.install << std::endl
					<< L"UninstallString: " << imf.unstall << std::endl << std::endl;
			}
		Lab_02:
			Namelen = MAX_PATH;
			RegCloseKey(valkey);
		}
		RegCloseKey(reskey);

		return ss;
	}

	void CommonMessageBack(SOCKET* tcp)
	{

	}
}