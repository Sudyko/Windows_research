#pragma once
#include <iostream>

class SystemInfo {  // the class that specify OS information
	std::string n_SystemVersion;
	std::string n_SystemEdition;
	std::string n_SystemServicePack;
	OSVERSIONINFOEX n_osvi;
	SYSTEM_INFO n_SysInfo;
	BOOL bOsVersionInfoEx;
private:
	void DetectVersion();
	void DetectEdition();
	void DetectServicePack();
	DWORD DetectProductInfo();
public:
	SystemInfo();
	//virtual ~SystemInfo();
	bool IsNTPlatform() const;
	bool IsWindowsPlatform() const;
	bool IsWindows32sPlatform() const;
	bool Is64bitPlatform() const;
	DWORD GetMajorVersion() const;
	DWORD GetMinorVersion() const;
	DWORD GetBuildNumber() const;
	std::string GetWindowsVersion() const;
	std::string GetWindowsEdition() const;
	std::string GetWindowsServicePack() const;
};