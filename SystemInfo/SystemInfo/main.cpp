#pragma warning(disable : 4996)

#include <string>
#include <Windows.h>
#include <tchar.h>
#include "./SystemInfo.h"

#define BUFSIZE 80

typedef BOOL(WINAPI* PGetProductInfo) (DWORD, DWORD, DWORD, DWORD, PDWORD);

SystemInfo::SystemInfo() {
    BOOL can_flag = true;
    LPSYSTEM_INFO pGNSI = NULL;

    memset(&n_osvi, 0, sizeof(OSVERSIONINFOEX));
    n_osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if (!(bOsVersionInfoEx = GetVersionEx(
        reinterpret_cast<OSVERSIONINFO*>(&n_osvi)))) {
        n_osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        if (!GetVersionEx((reinterpret_cast<OSVERSIONINFO*>(&n_osvi)))) {
            can_flag = false;
        }
    }

    pGNSI = reinterpret_cast<LPSYSTEM_INFO>(
        GetProcAddress(
            GetModuleHandle(_T("kernel32.dll")),
            "GetNativeSystemInfo"));
    if (pGNSI) n_SysInfo = (*pGNSI);
    else
        GetSystemInfo(&n_SysInfo);

    if (can_flag) {
        DetectVersion();
        DetectEdition();
        DetectServicePack();
    }
}

void SystemInfo::DetectVersion() {
    if (bOsVersionInfoEx) {
        switch (n_osvi.dwPlatformId) {
        case VER_PLATFORM_WIN32s:
            n_SystemVersion = "Windows32s";
            break;
        case VER_PLATFORM_WIN32_WINDOWS:
        {
            switch (n_osvi.dwMajorVersion) {
            case 4:
            {
                switch (n_osvi.dwMinorVersion) {
                case 0:
                {
                    if (n_osvi.szCSDVersion[0] == 'C' ||
                        n_osvi.szCSDVersion[0] == 'B')
                        n_SystemVersion = "Windows 95 R2";
                    else
                        n_SystemVersion = "Windows 95";
                    break;
                }
                case 10:
                {
                    if (n_osvi.szCSDVersion[0] == 'A')
                        n_SystemVersion = "Windows 98 SE";
                    else
                        n_SystemVersion = "Windows 98";
                    break;
                }
                case 90:
                    n_SystemVersion = "Windows ME";
                    break;
                }
            }
            break;
            }
        }
        case VER_PLATFORM_WIN32_NT:
        {
            switch (n_osvi.dwMajorVersion) {
            case 3:
                n_SystemVersion = "WindowsNT 3.51";
                break;
            case 4:
                switch (n_osvi.wProductType) {
                case 1:
                    n_SystemVersion = "WindowsNT 4.0 Workstation";
                    break;
                case 3:
                    n_SystemVersion = "WindowsNT 4.0 Server";
                    break;
                }
                break;
            case 5:
            {
                switch (n_osvi.dwMinorVersion) {
                case 0:
                    n_SystemVersion = "Windows 2000";
                    break;
                case 1:
                    n_SystemVersion = "Windows XP";
                    break;
                case 2:
                {
                    if (n_osvi.wSuiteMask == 0x8000)
                        n_SystemVersion = "Windows Home Server";
                    else if (n_osvi.wProductType == VER_NT_WORKSTATION &&
                        n_SysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
                        n_SystemVersion = "Windows XP Professional x64";
                    else {
                        n_SystemVersion = GetSystemMetrics(SM_SERVERR2) == 0 ?
                            "Windows Server 2003" :
                            "windows Server 2003 R2";
                    }
                }
                break;
                }
            }
            break;
            case 6:
            {
                switch (n_osvi.dwMinorVersion) {
                case 0:
                    n_SystemVersion = n_osvi.wProductType == VER_NT_WORKSTATION ?
                        "Windows Vista" :
                        "Windows Server 2008";
                    break;
                case 1:
                    n_SystemVersion = n_osvi.wProductType == VER_NT_WORKSTATION ?
                        "Windows 7" :
                        "Windows Server 2008 R2";
                    break;
                case 2:
                    n_SystemVersion = n_osvi.wProductType == VER_NT_WORKSTATION ?
                        "Windows 8" :
                        "Windows Server 2012";
                    break;
                case 3:
                    n_SystemVersion = n_osvi.wProductType == VER_NT_WORKSTATION ?
                        "Windows 8.1" :
                        "Windows Server 2012 R2";
                    break;
                }
            }
            break;
            case 10:
            {
                n_SystemVersion = n_osvi.wProductType == VER_NT_WORKSTATION ?
                    "Windows 10" :
                    "Windows Server 2016";
            }
            break;
            }
        }
        break;
        }
    } else {
        HKEY hKey;
        char szProductType[BUFSIZE];
        DWORD dwBufLen = BUFSIZE;
        LONG lRet;

        lRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            reinterpret_cast<LPCWSTR>(
                "SYSTEM\\CurrentControlSet\\Control\\ProductOptions"),
            0, KEY_QUERY_VALUE, &hKey);
        if (lRet != ERROR_SUCCESS)
            return;

        lRet = RegQueryValueEx(hKey, reinterpret_cast<LPCWSTR>(
            "ProductType"),
            NULL, NULL,
            (LPBYTE)szProductType, &dwBufLen);
        if ((lRet != ERROR_SUCCESS) || (dwBufLen > BUFSIZE))
            return;

        RegCloseKey(hKey);

        if (lstrcmpi(reinterpret_cast<LPCWSTR>("WINNT"),
            reinterpret_cast<LPCWSTR>(szProductType)) == 0) {
            if (n_osvi.dwMajorVersion <= 4) {
                n_SystemVersion = "WindowsNT40";
                n_SystemEdition = "Workstation";
            }
        }
        if (lstrcmpi(reinterpret_cast<LPCWSTR>("LANMANNT"),
            reinterpret_cast<LPCWSTR>(szProductType)) == 0) {
            if (n_osvi.dwMajorVersion == 5 && n_osvi.dwMinorVersion == 2)
                n_SystemVersion = "WindowsServer2003";

            if (n_osvi.dwMajorVersion == 5 && n_osvi.dwMinorVersion == 0) {
                n_SystemVersion = "Windows2000";
                n_SystemEdition = "Server";
            }

            if (n_osvi.dwMajorVersion <= 4) {
                n_SystemVersion = "WindowsNT40";
                n_SystemEdition = "Server";
            }
        }
        if (lstrcmpi(reinterpret_cast<LPCWSTR>("SERVERNT"),
            reinterpret_cast<LPCWSTR>(szProductType)) == 0) {
            if (n_osvi.dwMajorVersion == 5 && n_osvi.dwMinorVersion == 2) {
                n_SystemVersion = "WindowsServer2003";
                n_SystemEdition = "EnterpriseServer";
            }

            if (n_osvi.dwMajorVersion == 5 && n_osvi.dwMinorVersion == 0) {
                n_SystemVersion = "Windows2000";
                n_SystemEdition = "AdvancedServer";
            }

            if (n_osvi.dwMajorVersion <= 4) {
                n_SystemVersion = "WindowsNT40";
                n_SystemEdition = "EnterpriseServer";
            }
        }
    }
}

void SystemInfo::DetectEdition() {
    if (bOsVersionInfoEx) {
        switch (n_osvi.dwMajorVersion) {
        case 4:
        {
            switch (n_osvi.wProductType) {
            case VER_NT_WORKSTATION:
                n_SystemEdition = "Workstation";
                break;
            case VER_NT_SERVER:
                n_SystemEdition = (n_osvi.wSuiteMask & VER_SUITE_ENTERPRISE) != 0 ?
                    "Enterprise Server" :
                    "Standard Server";
                break;
            }
        }
        break;
        case 5:
        {
            switch (n_osvi.wProductType) {
            case VER_NT_WORKSTATION:
                n_SystemEdition = (n_osvi.wSuiteMask & VER_SUITE_PERSONAL) != 0 ?
                    "Home" :
                    "Professional";
                break;
            case VER_NT_SERVER:
            {
                switch (n_osvi.dwMinorVersion) {
                case 0:
                {
                    if ((n_osvi.wSuiteMask & VER_SUITE_DATACENTER) != 0)
                        n_SystemEdition = "Data Center Server";
                    else if ((n_osvi.wSuiteMask & VER_SUITE_ENTERPRISE) != 0)
                        n_SystemEdition = "Advanced Server";
                    else
                        n_SystemEdition = "Server";
                }
                break;
                default:
                {
                    if ((n_osvi.wSuiteMask & VER_SUITE_DATACENTER) != 0)
                        n_SystemEdition = "Data Center Server";
                    else if ((n_osvi.wSuiteMask & VER_SUITE_ENTERPRISE) != 0)
                        n_SystemEdition = "Enterprise Server";
                    else if ((n_osvi.wSuiteMask & VER_SUITE_BLADE) != 0)
                        n_SystemEdition = "Web Server";
                    else
                        n_SystemEdition = "Standard Server";
                }
                break;
                }
            }
            break;
            }
        }
        break;
        case 6: case 10:
        {
            DWORD dwReturnedProductType = DetectProductInfo();
            switch (dwReturnedProductType) {
            case PRODUCT_BUSINESS:
                n_SystemEdition = "Business";
                break;
            case PRODUCT_BUSINESS_N:
                n_SystemEdition = "Business N";
                break;
            case PRODUCT_CLUSTER_SERVER:
                n_SystemEdition = "HPC Edition";
                break;
            case PRODUCT_DATACENTER_SERVER:
                n_SystemEdition = "Server Datacenter (full installation)";
                break;
            case PRODUCT_DATACENTER_SERVER_CORE:
                n_SystemEdition = "Server Datacenter (core installation)";
                break;
            case PRODUCT_DATACENTER_SERVER_CORE_V:
                n_SystemEdition = "Server Datacenter without Hyper-V (core installation)";
                break;
            case PRODUCT_DATACENTER_SERVER_V:
                n_SystemEdition = "Server Datacenter without Hyper-V (full installation)";
                break;
            case PRODUCT_ENTERPRISE_SERVER:
                n_SystemEdition = "Server Enterprise (full installation)";
                break;
            case PRODUCT_ENTERPRISE_SERVER_CORE:
                n_SystemEdition = "Server Enterprise (core installation)";
                break;
            case PRODUCT_ENTERPRISE_SERVER_CORE_V:
                n_SystemEdition = "Server Enterprise without Hyper-V (core installation)";
                break;
            case PRODUCT_ENTERPRISE_SERVER_IA64:
                n_SystemEdition = "Server Enterprise for Itanium-based Systems";
                break;
            case PRODUCT_ENTERPRISE_SERVER_V:
                n_SystemEdition = "Server Enterprise without Hyper-V (full installation)";
                break;
            case PRODUCT_HOME_BASIC:
                n_SystemEdition = "Home Basic";
                break;
            case PRODUCT_HOME_BASIC_N:
                n_SystemEdition = "Home Basic N";
                break;
            case PRODUCT_HOME_PREMIUM:
                n_SystemEdition = "Home Premium";
                break;
            case PRODUCT_HOME_PREMIUM_N:
                n_SystemEdition = "Home Premium N";
                break;
            case PRODUCT_HOME_PREMIUM_SERVER:
                n_SystemEdition = "Windows Home Server 2011";
                break;
            case PRODUCT_HOME_SERVER:
                n_SystemEdition = "Windows Storage Server 2008 R2 Essentials";
                break;
            case PRODUCT_HYPERV:
                n_SystemEdition = "Microsoft Hyper-V Server";
                break;
            case PRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
                n_SystemEdition = "Windows Essential Business Server Management Server";
                break;
            case PRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
                n_SystemEdition = "Windows Essential Business Server Messaging Server";
                break;
            case PRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
                n_SystemEdition = "Windows Essential Business Server Security Server";
                break;
            case PRODUCT_SB_SOLUTION_SERVER:
                n_SystemEdition = "Windows Small Business Server 2011 Essentials";
                break;
            case PRODUCT_SERVER_FOR_SMALLBUSINESS:
                n_SystemEdition = "Windows Server 2008 for Windows Essential Server Solutions";
                break;
            case PRODUCT_SERVER_FOR_SMALLBUSINESS_V:
                n_SystemEdition = "Windows Server 2008 without Hyper-V for Windows Essential Server Solutions";
                break;
            case PRODUCT_SERVER_FOUNDATION:
                n_SystemEdition = "Server Foundation";
                break;
            case PRODUCT_SMALLBUSINESS_SERVER:
                n_SystemEdition = "Windows Small Business Server";
                break;
            case PRODUCT_SOLUTION_EMBEDDEDSERVER:
                n_SystemEdition = "Windows MultiPoint Server";
                break;
            case PRODUCT_STANDARD_SERVER:
                n_SystemEdition = "Server Standard (full installation)";
                break;
            case PRODUCT_STANDARD_SERVER_CORE:
                n_SystemEdition = "Server Standard (core installation)";
                break;
            case PRODUCT_STANDARD_SERVER_CORE_V:
                n_SystemEdition = "Server Standard without Hyper-V (core installation)";
                break;
            case PRODUCT_STANDARD_SERVER_V:
                n_SystemEdition = "Server Standard without Hyper-V (full installation)";
                break;
            case PRODUCT_STARTER:
                n_SystemEdition = "Starter";
                break;
            case PRODUCT_STARTER_N:
                n_SystemEdition = "Starter N";
                break;
            case PRODUCT_STORAGE_ENTERPRISE_SERVER:
                n_SystemEdition = "Storage Server Enterprise";
                break;
            case PRODUCT_STORAGE_EXPRESS_SERVER:
                n_SystemEdition = "Storage Server Express";
                break;
            case PRODUCT_STORAGE_STANDARD_SERVER:
                n_SystemEdition = "Storage Server Standard";
                break;
            case PRODUCT_STORAGE_WORKGROUP_SERVER:
                n_SystemEdition = "Storage Server Workgroup";
                break;
            case PRODUCT_ULTIMATE:
                n_SystemEdition = "Ultimate";
                break;
            case PRODUCT_ULTIMATE_N:
                n_SystemEdition = "Ultimate N";
                break;
            case PRODUCT_UNDEFINED:
                n_SystemEdition = "An unknown product";
                break;
            case PRODUCT_WEB_SERVER:
                n_SystemEdition = "Web Server (full installation)";
                break;
            case PRODUCT_WEB_SERVER_CORE:
                n_SystemEdition = "Web Server (core installation)";
                break;
#if _WIN32_WINNT >= 0x0601 && _WIN32_WINNT <= 0x0603  // Windows 7 and Windows 8
            case PRODUCT_ENTERPRISE:
                n_SystemEdition = "Enterprise";
                break;
            case PRODUCT_ENTERPRISE_N:
                n_SystemEdition = "Enterprise N";
                break;
            case PRODUCT_PROFESSIONAL:
                n_SystemEdition = "Professional";
                break;
            case PRODUCT_PROFESSIONAL_N:
                n_SystemEdition = "Professional N";
                break;
#endif  // _WIN32_WINNT >= 0x0601 && _WIN32_WINNT <= 0x0603
#if _WIN32_WINNT >= 0x0602 && _WIN32_WINNT <= 0x0603  // Windows 8 and Windows 8.1
            case PRODUCT_CORE:
                n_SystemEdition = "Windows 8";
                break;
            case PRODUCT_CORE_N:
                n_SystemEdition = "Windows 8 N";
                break;
            case PRODUCT_CORE_COUNTRYSPECIFIC:
                n_SystemEdition = "Windows 8 China";
                break;
            case PRODUCT_CORE_SINGLELANGUAGE:
                n_SystemEdition = "Windows 8 Single Language";
                break;
            case PRODUCT_ENTERPRISE_N_EVALUATION:
                n_SystemEdition = "Enterprise N (evaluation installation)";
                break;
            case PRODUCT_ENTERPRISE_EVALUATION:
                n_SystemEdition = "Server Enterprise (evaluation installation)";
                break;
#endif // _WIN32_WINNT >= 0x0602 && _WIN32_WINNT <= 0x0603
#if _WIN32_WINNT >= 0x0602 && _WIN32_WINNT <= 0x0A00  // Windows 8 and Windows 10
            case PRODUCT_CLUSTER_SERVER_V:
                n_SystemEdition = "Server Hyper Core V";
                break;
            case PRODUCT_DATACENTER_EVALUATION_SERVER:
                n_SystemEdition = "Server Datacenter (evaluation installation)";
                break;
            case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMT:
                n_SystemEdition = "Windows Essential Server Solution Management";
                break;
            case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDL:
                n_SystemEdition = "Windows Essential Server Solution Additional";
                break;
            case PRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC:
                n_SystemEdition = "Windows Essential Server Solution Management SVC";
                break;
            case PRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC:
                n_SystemEdition = "Windows Essential Server Solution Additional SVC";
                break;
            case PRODUCT_MULTIPOINT_STANDARD_SERVER:
                n_SystemEdition = "Windows MultiPoint Server Standard";
                break;
            case PRODUCT_MULTIPOINT_PREMIUM_SERVER:
                n_SystemEdition = "Windows MultiPoint Server Premium";
                break;
            case PRODUCT_PROFESSIONAL_WMC:
                n_SystemEdition = "Professional with Media Center";
                break;
            case PRODUCT_SB_SOLUTION_SERVER_EM:
                n_SystemEdition = "Server For SB Solutions EM";
                break;
            case PRODUCT_SERVER_FOR_SB_SOLUTIONS:
                n_SystemEdition = "Server For SB Solutions";
                break;
            case PRODUCT_SERVER_FOR_SB_SOLUTIONS_EM:
                n_SystemEdition = "Server For SB Solutions EM";
                break;
            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                n_SystemEdition = "Small Business Server Premium";
                break;
            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE:
                n_SystemEdition = "Small Business Server Premium (core installation)";
                break;
            case PRODUCT_STANDARD_EVALUATION_SERVER:
                n_SystemEdition = "Server Standard (evaluation installation)";
                break;
            case PRODUCT_STANDARD_SERVER_SOLUTIONS:
                n_SystemEdition = "Server Solutions Premium";
                break;
            case PRODUCT_STANDARD_SERVER_SOLUTIONS_CORE:
                n_SystemEdition = "Server Solutions Premium (core installation)";
                break;
            case PRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
                n_SystemEdition = "Storage Server Enterprise (core installation)";
                break;
            case PRODUCT_STORAGE_EXPRESS_SERVER_CORE:
                n_SystemEdition = "Storage Server Express (core installation)";
                break;
            case PRODUCT_STORAGE_STANDARD_EVALUATION_SERVER:
                n_SystemEdition = "Storage Server Standard (evaluation installation)";
                break;
            case PRODUCT_STORAGE_STANDARD_SERVER_CORE:
                n_SystemEdition = "Storage Server Standard (core installation)";
                break;
            case PRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER:
                n_SystemEdition = "Storage Server Workgroup (evaluation installation)";
                break;
            case PRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
                n_SystemEdition = "Storage Server Workgroup (core installation)";
                break;
#endif // _WIN32_WINNT >= 0x0602 && _WIN32_WINNT <= 0x0A00
#if _WIN32_WINNT == 0x0A00  // Windows 10
            case PRODUCT_CORE:
                n_SystemEdition = "Windows 10 Home";
                break;
            case PRODUCT_CORE_COUNTRYSPECIFIC:
                n_SystemEdition = "Windows 10 Home China";
                break;
            case PRODUCT_CORE_N:
                n_SystemEdition = "Windows 10 Home N";
                break;
            case PRODUCT_CORE_SINGLELANGUAGE:
                n_SystemEdition = "Windows 10 Home Single Language";
                break;
            case PRODUCT_DATACENTER_A_SERVER_CORE:
                n_SystemEdition = "Server Datacenter, Semi-Annual";
                break;
            case PRODUCT_STANDARD_A_SERVER_CORE:
                n_SystemEdition = "Server Standard, Semi-Annual Channel";
                break;
            case PRODUCT_EDUCATION:
                n_SystemEdition = "Windows 10 Education";
                break;
            case PRODUCT_EDUCATION_N:
                n_SystemEdition = "Windows 10 Education N";
                break;
            case PRODUCT_ENTERPRISE:
                n_SystemEdition = "Windows 10 Enterprise";
                break;
            case PRODUCT_ENTERPRISE_E:
                n_SystemEdition = "Windows 10 Enterprise E";
                break;
            case PRODUCT_ENTERPRISE_EVALUATION:
                n_SystemEdition = "Windows 10 Enterprise Evaluation";
                break;
            case PRODUCT_ENTERPRISE_N:
                n_SystemEdition = "Windows 10 Enterprise N";
                break;
            case PRODUCT_ENTERPRISE_N_EVALUATION:
                n_SystemEdition = "Windows 10 Enterprise N Evaluation";
                break;
            case PRODUCT_ENTERPRISE_S:
                n_SystemEdition = "Windows 10 Enterprise 2015 LTSB";
                break;
            case PRODUCT_ENTERPRISE_S_EVALUATION:
                n_SystemEdition = "Windows 10 Enterprise 2015 LTSB Evaluation";
                break;
            case PRODUCT_ENTERPRISE_S_N:
                n_SystemEdition = "Windows 10 Enterprise 2015 LTSB N";
                break;
            case PRODUCT_ENTERPRISE_S_N_EVALUATION:
                n_SystemEdition = "Windows 10 Enterprise 2015 LTSB N Evaluation";
                break;
            case PRODUCT_IOTUAP:
                n_SystemEdition = "Windows 10 IoT Core";
                break;
            case PRODUCT_PRO_WORKSTATION:
                n_SystemEdition = "Windows 10 Pro for Workstations";
                break;
            case PRODUCT_PRO_WORKSTATION_N:
                n_SystemEdition = "Windows 10 Pro for Workstations N";
                break;
            case PRODUCT_PROFESSIONAL:
                n_SystemEdition = "Windows 10 Pro";
                break;
            case PRODUCT_PROFESSIONAL_N:
                n_SystemEdition = "Windows 10 Pro N";
                break;
#endif // _WIN32_WINNT == 0x0A00
            }
        }
        break;
        }
    }
}

DWORD SystemInfo::DetectProductInfo() {
    DWORD dwProductInfo = PRODUCT_UNDEFINED;
#if _WIN32_WINNT >= 0x0600  // Windows Vista and later
    if (n_osvi.dwMajorVersion >= 6) {
        PGetProductInfo lpProducInfo = reinterpret_cast<PGetProductInfo>(
            GetProcAddress(GetModuleHandle(_T("kernel32.dll")),
                "GetProductInfo"));
        if (lpProducInfo != NULL) {
            lpProducInfo(n_osvi.dwMajorVersion,
                n_osvi.dwMinorVersion,
                n_osvi.wServicePackMajor,
                n_osvi.wServicePackMinor,
                &dwProductInfo);
        }
    }
#endif  // _WIN32_WINNT >= 0x0600
    return dwProductInfo;
}

void SystemInfo::DetectServicePack() {
    if (n_osvi.wServicePackMajor) {
        n_SystemServicePack = "Service pack ";
        std::string lhs_pack, rhs_pack;
        lhs_pack = std::to_string(n_osvi.wServicePackMajor);
        rhs_pack = std::to_string(n_osvi.wServicePackMinor);
        n_SystemServicePack += (lhs_pack + "." + rhs_pack);
    }
    else
        n_SystemServicePack = "Service Pack: None";
}

std::string SystemInfo::GetWindowsVersion() const {
    return n_SystemVersion;
}

std::string SystemInfo::GetWindowsEdition() const {
    return n_SystemEdition;
}

std::string SystemInfo::GetWindowsServicePack() const {
    return n_SystemServicePack;
}

bool SystemInfo::IsNTPlatform() const {
    return n_osvi.dwPlatformId == VER_PLATFORM_WIN32_NT;
}

bool SystemInfo::IsWindowsPlatform() const {
    return n_osvi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS;
}

bool SystemInfo::IsWindows32sPlatform() const {
    return n_osvi.dwPlatformId == VER_PLATFORM_WIN32s;
}

DWORD SystemInfo::GetMajorVersion() const {
    return n_osvi.dwMajorVersion;
}

DWORD SystemInfo::GetMinorVersion() const {
    return n_osvi.dwMinorVersion;
}

DWORD SystemInfo::GetBuildNumber() const {
    return n_osvi.dwBuildNumber;
}

bool SystemInfo::Is64bitPlatform() const {
    if (GetSystemWow64DirectoryW(nullptr, 0u)) {
        auto const last_error = GetLastError();
        if (last_error == ERROR_CALL_NOT_IMPLEMENTED)
            return false;
    }
    return true;
}

int main() {
	SystemInfo sysInfo;
    TCHAR szServicePack[128] = { 0 };

    std::cout << "Version: " << sysInfo.GetWindowsVersion() << "\n" <<
        "Edition: " << sysInfo.GetWindowsEdition() << "\n" <<
        sysInfo.GetWindowsServicePack() << "\n" <<
        "Build Number: " << sysInfo.GetBuildNumber() << "\n" <<
        "Major and Minor Versions: " << sysInfo.GetMajorVersion() <<
        "." << sysInfo.GetMinorVersion() << "\n" << "Platform: ";
    if (sysInfo.IsNTPlatform())
        std::cout << "NT" << std::endl;
    else if (sysInfo.IsWindowsPlatform())
        std::cout << "Windows" << std::endl;
    else if (sysInfo.IsWindows32sPlatform())
        std::cout << "Windows 32s" << std::endl;
    else
        std::cout << "Unknown" << std::endl;
    if (sysInfo.Is64bitPlatform())
        std::cout << "x64" << std::endl;
    else
        std::cout << "x32" << std::endl;
	return 0;
}