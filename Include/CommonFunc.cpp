#include "StdAfx.h"
#include "CommonFunc.h"
#include "Wininet.h"
#include <Tlhelp32.h>

#pragma comment(lib, "Wininet.lib")


CCommonFunc::CCommonFunc(void)
{

}

CCommonFunc::~CCommonFunc(void)
{
}

float CCommonFunc::GetPreciseDeltaTime(LARGE_INTEGER startTime, LARGE_INTEGER endTime)
{
	LARGE_INTEGER timer;
	float DeltaTime;

	QueryPerformanceFrequency(&timer);

	return DeltaTime = (endTime.QuadPart - startTime.QuadPart) / (float)timer.QuadPart;
}

UINT CCommonFunc::GetTimerCount()
{
	return g_nTimerCount;
}

void CCommonFunc::IncreaseTimerCount()
{
	++g_nTimerCount;
}

UINT CCommonFunc::GetCaptureCount()
{
	return g_nCaptureCount;
}

void CCommonFunc::IncreaseCaptureCount()
{
	if(g_nTimerCount > g_nCaptureCount)
		++g_nCaptureCount;
}

void CCommonFunc::DecreaseCaptureCount()
{
	--g_nCaptureCount;
}

INT CCommonFunc::GetPassCount()
{
	return g_nPassCount;
}

void CCommonFunc::IncreasePassCount()
{
	++g_nPassCount;
}

void CCommonFunc::DecreasePassCount()
{
	--g_nPassCount;
}

INT CCommonFunc::GetTrashCount()
{
	return g_nTrashCount;
}

void CCommonFunc::IncreaseTrashCount()
{
	++g_nTrashCount;
}

void CCommonFunc::DecreaseTrashCount()
{
	--g_nTrashCount;
}

void CCommonFunc::SetFaultRatio(BOOL bValue)
{
	g_bFaultRatio = bValue;
}

BOOL CCommonFunc::GetFaultRatio()
{
	return g_bFaultRatio;
}

void CCommonFunc::SetProgramType(INT nType)
{
	g_nProgramType = nType;
}

INT CCommonFunc::GetProgramType()
{
	return g_nProgramType;
}

void CCommonFunc::SetDivValue(double dValue)
{
	g_dDivValue = dValue;
}

double CCommonFunc::GetDivValue()
{
	return g_dDivValue;
}

void CCommonFunc::SetRealDivValue(double dValue)
{
	g_dRealDivValue = dValue;
}

double CCommonFunc::GetRealDivValue()
{
	return g_dRealDivValue;
}

void CCommonFunc::SetReverseDivValue(double dValue)
{
	g_dReverseDivValue = dValue;
}

double CCommonFunc::GetReverseDivValue()
{
	return g_dReverseDivValue;
}

INT CCommonFunc::GetRealSize(INT nValue)
{
	double dValue = g_dDivValue - 1.0;
	if(dValue == 0.0)
		return nValue;
	nValue -= (INT) ((double)nValue * dValue);
	return nValue;
}

CHAR* CCommonFunc::WcharToUtf8(WCHAR* wszText)
{
	int l = WideCharToMultiByte(CP_UTF8, 0, wszText, -1, 0, 0, 0, 0);
	WideCharToMultiByte(CP_UTF8, 0, wszText, -1, g_szChar, CHAR_LEN_SIZE, 0, 0);
	return g_szChar;
}

CHAR* CCommonFunc::WCharToChar(WCHAR* wszText)
{
	int l = WideCharToMultiByte(CP_ACP, 0, wszText, -1, 0, 0, 0, 0);
	WideCharToMultiByte(CP_ACP, 0, wszText, -1, g_szChar, CHAR_LEN_SIZE, 0, 0);
	return g_szChar;
}

CHAR* CCommonFunc::Utf8ToChar(char* str)
{
	int nLen=0;
	//UTF8 to Unicode(rbuff의 문자열을 Unicode로 변환하여 Uni에 저장)
	nLen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0); 
	MultiByteToWideChar(CP_UTF8, 0, str, -1, g_wszChar, nLen);  
	//Unicode to Ansi(Uni의 문자열을 ANSI로 변환하여 다시 rbuff에 저장)
	nLen = WideCharToMultiByte(CP_ACP, 0, g_wszChar, -1, NULL, 0, NULL, NULL);
	WideCharToMultiByte(CP_ACP, 0, g_wszChar, -1, g_szChar, nLen, NULL, NULL);

	return g_szChar;
}

TCHAR* CCommonFunc::CharToTCHAR(char *str)
{
#if defined(UNICODE) || defined(_UNICODE)
        int nLen = strlen(str) + 1;
        memset(g_tszChar, 0x00, nLen*sizeof(TCHAR));
        MultiByteToWideChar(CP_ACP, 0, str, -1, g_tszChar, nLen*sizeof(TCHAR));
#else
        int nLen = strlen(str) + 1;
        memset(g_tszChar, 0x00, nLen*sizeof(TCHAR));
        _tcscpy(g_tszChar, str);
#endif
        return g_tszChar;
}

WCHAR* CCommonFunc::CharToWCHAR(char *str)
{
    int nLen = strlen(str) + 1;
    memset(g_wszChar, 0x00, nLen*sizeof(WCHAR));
    MultiByteToWideChar(CP_ACP, 0, str, -1, g_wszChar, nLen*sizeof(WCHAR));

	return g_wszChar;
}

WCHAR* CCommonFunc::Utf8ToWCHAR(char* str)
{
	int nLen = 0;
	//UTF8 to Unicode(rbuff의 문자열을 Unicode로 변환하여 Uni에 저장)
	nLen = MultiByteToWideChar(CP_UTF8, 0, str, -1, NULL, 0);
	MultiByteToWideChar(CP_UTF8, 0, str, -1, g_wszChar, nLen);

	return g_wszChar;
}

void CCommonFunc::GetMonitorInfoEx(CRect & rect)
{
	rect.SetRect(0, 0, 0, 0);
	int MonitorCount = GetSystemMetrics(SM_CMONITORS);//모니터수얻어오기
	DISPLAY_DEVICE dd;
	DEVMODE dm;

	//모니터번호와 디바이스번호는다를수있음
	for (int i=0,j=0;j<MonitorCount;i++)
	{
		memset(&dd, 0, sizeof(DISPLAY_DEVICE));
		dd.cb = sizeof(DISPLAY_DEVICE);
		//디스플레이정보얻기
		EnumDisplayDevices(NULL, i, &dd, 0);
		memset(&dm, 0, sizeof(DEVMODE));
		//디스플레이설정값얻어오기
		if(EnumDisplaySettings(dd.DeviceName,ENUM_CURRENT_SETTINGS,&dm))
		{
			if(rect.left > dm.dmPosition.x) rect.left = dm.dmPosition.x;
			if(rect.top > dm.dmPosition.y) rect.top = dm.dmPosition.y;
			if(rect.right < dm.dmPosition.x+dm.dmPelsWidth) rect.right = dm.dmPosition.x+dm.dmPelsWidth;
			if(rect.bottom < dm.dmPosition.y+dm.dmPelsHeight) rect.bottom = dm.dmPosition.y+dm.dmPelsHeight;
			j++;
		}
	}
}

void CCommonFunc::GetMonitorInfo(CRect & rect, INT nType) // 0: 주모니터, 1: 전체 모니터
{
	// 메인 모니터 크기
	int width = GetSystemMetrics(SM_CXSCREEN);
	int height = GetSystemMetrics(SM_CYSCREEN);
	// 듀얼 모니터 사용시 전체화면
	int nMonitorWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	int nMonitorHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);

	switch(nType)
	{
	case 0: rect.SetRect(0, 0, width, height); break;
	case 1: rect.SetRect(0, 0, nMonitorWidth, nMonitorHeight); break;
	}
}

CString CCommonFunc::GetCurrentOsVersionString()
{
	CString sVersion;
	GetCurrentOsVersion(sVersion);
	return sVersion;
}

typedef void (WINAPI * RtlGetVersion_FUNC) (OSVERSIONINFOEXW *);
BOOL GetVersion(OSVERSIONINFOEX * os) {
    HMODULE hMod;
    RtlGetVersion_FUNC func;
#ifdef UNICODE
    OSVERSIONINFOEXW * osw = os;
#else
    OSVERSIONINFOEXW o;
    OSVERSIONINFOEXW * osw = &o;
#endif

    hMod = LoadLibrary(TEXT("ntdll.dll"));
    if (hMod) {
        func = (RtlGetVersion_FUNC)GetProcAddress(hMod, "RtlGetVersion");
        if (func == 0) {
            FreeLibrary(hMod);
            return FALSE;
        }
        ZeroMemory(osw, sizeof (*osw));
        osw->dwOSVersionInfoSize = sizeof (*osw);
        func(osw);
#ifndef UNICODE
        os->dwBuildNumber = osw->dwBuildNumber;
        os->dwMajorVersion = osw->dwMajorVersion;
        os->dwMinorVersion = osw->dwMinorVersion;
        os->dwPlatformId = osw->dwPlatformId;
        os->dwOSVersionInfoSize = sizeof (*os);
        DWORD sz = sizeof (os->szCSDVersion);
        WCHAR * src = osw->szCSDVersion;
        unsigned char * dtc = (unsigned char *)os->szCSDVersion;
        while (*src)
            * Dtc++ = (unsigned char)* src++;
        *Dtc = '\ 0';
#endif

    } else
        return FALSE;
    FreeLibrary(hMod);
    return TRUE;
}
double CCommonFunc::GetCurrentOsVersion(CString & sVersion)
{
	if(g_dOSVersion != -1)
	{
		sVersion = g_sOsVersion;
		return g_dOSVersion;
	}
    OSVERSIONINFOEX osVersionInfoEx;
     
    ZeroMemory(&osVersionInfoEx, sizeof(OSVERSIONINFOEXW));
    osVersionInfoEx.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

	if(!GetVersion(&osVersionInfoEx))
		GetVersionExW((LPOSVERSIONINFOW)&osVersionInfoEx);
 
    switch (osVersionInfoEx.dwMajorVersion)
    {
    case 5:
    {
        switch (osVersionInfoEx.dwMinorVersion)
        {
		case 0:
            sVersion = _T("Windows 2000");
			g_dOSVersion = 50;
            break;
        case 1:
            sVersion = _T("Windows XP");
			g_dOSVersion = 51;
            break;
        case 2:
            if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows XP Professional x64 Edition");
				g_dOSVersion = 52.1;
			}
            else if (GetSystemMetrics(SM_SERVERR2) == 0)
			{
                sVersion = _T("Windows Server 2003");
				g_dOSVersion = 52.2;
			}
            else if (osVersionInfoEx.wSuiteMask & VER_SUITE_WH_SERVER)
			{
                sVersion = _T("Windows Home Server");
				g_dOSVersion = 52.3;
			}
            else if (GetSystemMetrics(SM_SERVERR2) != 0)
			{
                sVersion = _T("Windows Server 2003 R2");
				g_dOSVersion = 52.4;
			}
            break;
        default:
            CString err;
            err.Format(_T("Can't find version name of dwMiniorVersion: %d"), osVersionInfoEx.dwMinorVersion);
            sVersion = err;
			g_dOSVersion = 53;
            break;
        }
        break;
    }
    case 6:
    {
        switch (osVersionInfoEx.dwMinorVersion)
        {
        case 0:
            if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows Vista");
				g_dOSVersion = 60.1;
			}
            else if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows Server 2008");
				g_dOSVersion = 60.2;
			}
            break;
        case 1:
            if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows Server 2008 R2");
				g_dOSVersion = 61.1;
			}
            else if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows 7");
				g_dOSVersion = 61.2;
			}
            break;
        case 2:
            if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows Server 2012");
				g_dOSVersion = 62.1;
			}
            else if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows 8");
				g_dOSVersion = 62.2;
			}
            break;
        case 3:
            if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows Server 2012 R2");
				g_dOSVersion = 63.1;
			}
            else if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows 8.1");
				g_dOSVersion = 63.2;
			}
            break;
        default:
            CString err;
            err.Format(_T("Can't find version name of dwMiniorVersion: %d"), osVersionInfoEx.dwMinorVersion);
            sVersion = err;
			g_dOSVersion = 64;
            break;
        }
    }
	break;
	case 10:
	{
		switch (osVersionInfoEx.dwMinorVersion)
        {
        case 0:
            if (osVersionInfoEx.wProductType == VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows 10");
				g_dOSVersion = 100.1;
			}
            else if (osVersionInfoEx.wProductType != VER_NT_WORKSTATION)
			{
                sVersion = _T("Windows Server 2016 Technical Preview");
				g_dOSVersion = 100.2;
			}
            break;
        default:
            CString err;
            err.Format(_T("Can't find version name of dwMiniorVersion: %d"), osVersionInfoEx.dwMinorVersion);
            sVersion = err;
			g_dOSVersion = 109;
            break;
		}
	}
	break;
    default:
		sVersion = _T("UNKNOWN Windows version.");
		g_dOSVersion = 200;
        break;
    }
	g_sOsVersion = sVersion;
	return g_dOSVersion;
}

// using undocumented functions and structures

#define SystemBasicInformation		0
#define	SystemPerformanceInformation	2
#define SystemTimeInformation		3

#define Li2Double(x)	((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))

typedef struct
{
	DWORD	dwUnknown1;
	ULONG	uKeMaximumIncrement;
	ULONG	uPageSize;
	ULONG	uMmNumberOfPhysicalPages;
	ULONG	uMmLowestPhysicalPage;
	ULONG	UMmHighestPhysicalPage;
	ULONG	uAllocationGranularity;
	PVOID	pLowestUserAddress;
	PVOID	pMmHighestUserAddress;
	ULONG	uKeActiveProcessors;
	BYTE	bKeNumberProcessors;
	BYTE	bUnknown2;
	WORD	bUnknown3;
} SYSTEM_BASIC_INFORMATION;

typedef struct
{
	LARGE_INTEGER	liIdleTime;
	DWORD		dwSpare[76];
} SYSTEM_PERFORMANCE_INFORMATION;

typedef struct
{
	LARGE_INTEGER	liKeBootTime;
	LARGE_INTEGER	liKeSystemTime;
	LARGE_INTEGER	liExpTimeZoneBias;
	ULONG			uCurrentTimeZoneID;
	DWORD			dwReserved;
} SYSTEM_TIME_INFORMATION;

typedef LONG (WINAPI *PROCNTQSI) (UINT, PVOID, ULONG, PULONG);

double CCommonFunc::GetCPUUsages()
{
	SYSTEM_BASIC_INFORMATION		SysBaseInfo;
	SYSTEM_TIME_INFORMATION			SysTimeInfo;
	SYSTEM_PERFORMANCE_INFORMATION	SysPerfInfo;
	LONG							status;
	LARGE_INTEGER					liOldIdleTime = {0, 0};
	LARGE_INTEGER					liOldSystemTime = {0, 0};
	double							dbIdleTime;
	double							dbSystemTime;
	PROCNTQSI						NtQuerySystemInformation;

	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle(L"ntdll"), "NtQuerySystemInformation");
	if (NtQuerySystemInformation)
	{
		status = NtQuerySystemInformation(SystemBasicInformation, &SysBaseInfo, sizeof(SysBaseInfo), NULL);
		if (status == NO_ERROR)
		{
			// get system time
			status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), NULL);
			if (status == NO_ERROR)
			{
				// get system idle time
				status = NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
				if (status == NO_ERROR)
				{
					liOldIdleTime = SysPerfInfo.liIdleTime;
					liOldSystemTime = SysTimeInfo.liKeSystemTime;
					// wait one second
					::Sleep(1000);
					// get new System time
					status = NtQuerySystemInformation(SystemTimeInformation, &SysTimeInfo, sizeof(SysTimeInfo), NULL);

					if (status == NO_ERROR)
					{
						// get new system idle time
						status = NtQuerySystemInformation(SystemPerformanceInformation, &SysPerfInfo, sizeof(SysPerfInfo), NULL);
						if (status == NO_ERROR)
						{
							// current value = new value - old value
							dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime);
							dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
							// currentCpuIdle = IdleTime / SystemTime;
							dbIdleTime = dbIdleTime / dbSystemTime;
							// currentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors
							dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
						}
					}
				}
			}
		}
	}

	return dbIdleTime;
}

INT CCommonFunc::GetInternetState()
{
	DWORD dwConnectionTypes;
	if(InternetGetConnectedState(&dwConnectionTypes, 0)) // 정상적으로 검사됨
	{
		if((dwConnectionTypes & INTERNET_CONNECTION_MODEM) != 0)
		{
			//TRACE("Modem으로 인터넷 사용중...");
			return 1;
		}
		if((dwConnectionTypes & INTERNET_CONNECTION_LAN) != 0)
		{
			//TRACE("LAN으로 인터넷 사용중...");
			return 2;
		}
		if((dwConnectionTypes & INTERNET_CONNECTION_PROXY) != 0)
		{
			//TRACE("Proxy로 인터넷 사용중...");
			return 3;
		}
		if((dwConnectionTypes & INTERNET_CONNECTION_MODEM_BUSY) != 0)
		{
			//TRACE("Modem을 다른 용도로 사용중");
			return 4;
		}
		if((dwConnectionTypes & INTERNET_RAS_INSTALLED) != 0)
		{
			//TRACE("RAS가 설치되어 있음");
			return 5;
		}
		if((dwConnectionTypes & INTERNET_CONNECTION_OFFLINE) != 0)
		{
			//TRACE("오프라인");
			return -2;
		}
		if((dwConnectionTypes & INTERNET_CONNECTION_CONFIGURED) != 0)
		{
			//TRACE("인터넷 연결이 설정되었음");
			return 7;
		}
	}
	else
	{
		//TRACE("검사할 수 없습니다");
		return -1;
	}
	return 0;
}

BOOL CCommonFunc::MixVoiceData(LPWORD lpRecvData, LPWORD lpRecordData, LPWORD lpDstData, INT nSize, INT bitsPerSample)
{
	try{
		long sampleSize = nSize / (bitsPerSample >> 3);
		switch(bitsPerSample)
		{
		case 8:
			{
				float gain = log10(20.0f);
				for (long i=0; i < sampleSize; i++)
				{
					*lpDstData = (BYTE)(((*lpRecvData + *lpRecordData)>>1) * gain);
					lpRecvData++;
					lpRecordData++;
				}
			}
			break;
		case 16:
			for (long i=0; i < sampleSize; i++)
			{
				float sample1 = (*lpRecvData - 32768) / 32768.0f;
				float sample2 = (*lpRecordData - 32768) / 32768.0f;
				if (fabs(sample1*sample2) > 0.25f)
					*lpDstData = (WORD)(*lpRecvData + *lpRecordData);
				else
					*lpDstData = fabs(sample1) < fabs(sample2) ? *lpRecvData : *lpRecordData;
				lpRecvData++;
				lpRecordData++;
				lpDstData++;
			}
			break;
		default:
			break;
		}
	}catch(...)
	{
		return TRUE;
	}
	return TRUE;
}

BOOL CCommonFunc::VoiceVolumeControl(LPWORD lpRecordData, INT nSize, INT bitsPerSample, float gain)
{
	try{
		long sampleSize = nSize / (bitsPerSample >> 3);
		switch(bitsPerSample)
		{
		case 16:
			for (long i=0; i < sampleSize; i++)
			{
				*lpRecordData = (WORD)(*lpRecordData * pow( 2.0f, gain * 0.05f ));
				lpRecordData++;
			}
			break;
		default:
			break;
		}
	}catch(...)
	{
		return TRUE;
	}
	return TRUE;
}

BOOL CCommonFunc::FindProcess(CString sExeName, HANDLE & hHandle)
{
	sExeName.MakeUpper();
	HANDLE hSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, 0 );
	if ( (int)hSnapshot != -1 )
	{
		PROCESSENTRY32 pe32 ;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		BOOL bContinue ;
		CString strProcessName; 
		DWORD dwPID = GetCurrentProcessId();
		if ( Process32First ( hSnapshot, &pe32 ) )
		{
			do
			{
				Sleep(1);
				if(dwPID == pe32.th32ProcessID)
				{
					bContinue = Process32Next ( hSnapshot, &pe32 ); 
					continue;
				}
				strProcessName = pe32.szExeFile; //strProcessName이 프로세스 이름;
				strProcessName.MakeUpper();
				if( ( strProcessName.Find(sExeName,0) != -1 ) )
				{
					hHandle = OpenProcess( PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID );
					CloseHandle( hSnapshot );
					if( hHandle )
						return TRUE;
					return FALSE;
				} 
				bContinue = Process32Next ( hSnapshot, &pe32 ); 
			} while ( bContinue );
		} 
		CloseHandle( hSnapshot );
	}
	return FALSE;
}

BOOL CCommonFunc::FindProcessID(CString sExeName, DWORD & pID)
{
	sExeName.MakeUpper();
	HANDLE hSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, 0 );
	if ( (int)hSnapshot != -1 )
	{
		PROCESSENTRY32 pe32 ;
		pe32.dwSize = sizeof(PROCESSENTRY32);
		BOOL bContinue ;
		CString strProcessName; 
		DWORD dwPID = GetCurrentProcessId();
		if ( Process32First ( hSnapshot, &pe32 ) )
		{
			do
			{
				Sleep(1);
				if(dwPID == pe32.th32ProcessID)
				{
					bContinue = Process32Next ( hSnapshot, &pe32 ); 
					continue;
				}
				strProcessName = pe32.szExeFile; //strProcessName이 프로세스 이름;
				strProcessName.MakeUpper();
				if( ( strProcessName.Find(sExeName,0) != -1 ) )
				{
					pID = pe32.th32ProcessID;
					//hHandle = OpenProcess( PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID );
					CloseHandle( hSnapshot );
					return TRUE;
					/*if( hHandle )
						return TRUE;
					return FALSE;*/
				} 
				bContinue = Process32Next ( hSnapshot, &pe32 ); 
			} while ( bContinue );
		} 
		CloseHandle( hSnapshot );
	}
	return FALSE;
}

BOOL CCommonFunc::ExecuteCreateWindow(CString strPath, HWND & hWnd)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si,sizeof(STARTUPINFO));
	ZeroMemory(&pi,sizeof(PROCESS_INFORMATION));

	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	si.lpDesktop = L"WinSta0\\Default"; 

	CreateProcess(strPath, NULL, NULL, NULL, FALSE, /*CREATE_NO_WINDOW*/0, NULL, NULL, &si, &pi);

	WaitForSingleObject( pi.hProcess, 3000);
	if(pi.hProcess)
	{
		DWORD dwExitCode = STILL_ACTIVE;
		while(dwExitCode == STILL_ACTIVE)
		{
			GetExitCodeProcess(pi.hProcess, &dwExitCode);
		} 
	}
	CloseHandle(pi.hProcess); 
	CloseHandle(pi.hThread);

	return TRUE;
}

BOOL CCommonFunc::GetTrayRect(CRect & rect)
{
	HWND hWnd = ::FindWindow(_T("Shell_TrayWnd"), NULL);
    if(hWnd)
	{
        ::GetWindowRect(hWnd, &rect);
		return TRUE;
    }
	rect.SetRect(0, 0, 0, 0);
	return FALSE;
}

BOOL CCommonFunc::MakeResource2File(CString strFilename, UINT uIDResource, LPCTSTR lpResourceType)
{
	HINSTANCE hInstance = AfxGetInstanceHandle();
	HRSRC hRsrc = FindResource(hInstance, MAKEINTRESOURCE(uIDResource), lpResourceType);
	if (hRsrc == NULL)
		return FALSE;
	HANDLE hRes = LoadResource(hInstance, hRsrc);
	if (hRes == NULL)
		return FALSE;
	LPSTR lpRes = (LPSTR)LockResource(hRes);
	if (lpRes == NULL)
		return FALSE;
 
	CFile file;
	if (!file.Open(strFilename, CFile::modeCreate|CFile::modeWrite|CFile::typeBinary|CFile::shareDenyNone))
		return FALSE;
	file.Write(lpRes, SizeofResource(hInstance, hRsrc));
	file.Close();
	//UnlockResource(hRes);
 
	return TRUE;
}

BOOL CCommonFunc::GetCurrentPath(CString & strPath)
{
	WCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);

	strPath = szPath;
	return TRUE;
}

CString CCommonFunc::GetCurrentPath()
{
	WCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);

	return szPath;
}

BOOL CCommonFunc::GetRealFilePath(CString sFileName, CString & sPath)
{
	WCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);

	sPath.Format(L"%s\\%s", szPath, sFileName);
	return TRUE;
}

CString CCommonFunc::GetMakePath(CString sName)
{
	CString sPath;
	WCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);

	sPath.Format(L"%s\\%s", szPath, sName);
	return sPath;
}

void CCommonFunc::GetMakeFolderPath(CString sFolderName, CString & sFolderPath)
{
	WCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);

	sFolderPath.Format(L"%s\\%s", szPath, sFolderName);
	CreateDirectory(sFolderPath);
}

BOOL CCommonFunc::CreateDirectory(LPCTSTR szDir)
{
	CString strTemp(szDir);

	if(strTemp.GetLength() <= 3)
		return FALSE;

	if(strTemp[strTemp.GetLength() - 1] == _T('\\'))
		strTemp = strTemp.Left(strTemp.GetLength() - 1);
	
	DWORD dwAttr = ::GetFileAttributes(strTemp);
	if(dwAttr == -1)
	{
		// 하위디렉토리를 만든 다음 디렉토리를 만든다.
		CreateDirectory(strTemp.Left(strTemp.ReverseFind(_T('\\'))));

		BOOL bSucc = ::CreateDirectory(strTemp, NULL);
		if (!bSucc && ::GetLastError() != ERROR_ALREADY_EXISTS)
			;
		else if(!bSucc)
			return FALSE;
	}
	else if(!(dwAttr & FILE_ATTRIBUTE_DIRECTORY))
	{
	}
	return TRUE;
}

void CCommonFunc::DeleteDirectory(LPCTSTR szDir)
{
	CString strDir(szDir);
	if(strDir[strDir.GetLength() - 1] != _T('\\'))
		strDir += _T("\\");

	CFileFind finder;
	BOOL bSucc = finder.FindFile(strDir + _T("*"));
	while(bSucc)
	{
		bSucc = finder.FindNextFile();
		
		CString fileName = finder.GetFileName();
		if(finder.IsDirectory())
		{
			if(fileName[fileName.GetLength() - 1] != _T('.'))
				DeleteDirectory(strDir + fileName + _T("\\"));
		}
		else
		{
			CString strFileName = strDir + fileName;
			
			::SetFileAttributes(strFileName, FILE_ATTRIBUTE_NORMAL);
			::DeleteFile(strFileName);
		}
 	}
	finder.Close();
	::RemoveDirectory(strDir);
}

void CCommonFunc::DeleteDataInDirectory(LPCTSTR szDir)
{
	CString strDir(szDir);
	if(strDir[strDir.GetLength() - 1] != _T('\\'))
		strDir += _T("\\");

	CFileFind finder;
	BOOL bSucc = finder.FindFile(strDir + _T("*"));
	while(bSucc)
	{
		bSucc = finder.FindNextFile();
		
		CString fileName = finder.GetFileName();
		if(finder.IsDirectory())
		{
			if(fileName[fileName.GetLength() - 1] != _T('.'))
				DeleteDirectory(strDir + fileName + _T("\\"));
		}
		else
		{
			CString strFileName = strDir + fileName;
			
			::SetFileAttributes(strFileName, FILE_ATTRIBUTE_NORMAL);
			::DeleteFile(strFileName);
		}
 	}
	finder.Close();
}

CString	CCommonFunc::GetCurrentModuleFileName()
{
	WCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	CString sPath = szPath;
	return sPath.Right(sPath.GetLength() - sPath.ReverseFind(L'\\'));
}

BOOL CCommonFunc::SaveBMPFile(BYTE* buffer, INT nWidth, INT nHeight, CHAR* szPath, INT nBitmapDepth)
{
	DWORD dwBmpSize = nWidth * nHeight * (nBitmapDepth/8) + 54;
	CHAR * pbBmpData = new char[dwBmpSize];
	BITMAPFILEHEADER bmHeader;
	BITMAPINFO       bmInfo;
	bmHeader.bfType = 'MB';
	bmHeader.bfSize = dwBmpSize;
	bmHeader.bfReserved1 = 0;
	bmHeader.bfReserved2 = 0;
	bmHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biSize   = sizeof(BITMAPINFOHEADER);
	bmInfo.bmiHeader.biWidth  = nWidth;
	bmInfo.bmiHeader.biHeight = -nWidth;
	bmInfo.bmiHeader.biPlanes = 1;
	bmInfo.bmiHeader.biBitCount = nBitmapDepth;
	bmInfo.bmiHeader.biCompression = BI_RGB;
	bmInfo.bmiHeader.biSizeImage   = dwBmpSize - 54;
	bmInfo.bmiHeader.biXPelsPerMeter = 0;
	bmInfo.bmiHeader.biYPelsPerMeter = 0;
	bmInfo.bmiHeader.biClrUsed = 0;
	bmInfo.bmiHeader.biClrImportant = 0;

	memcpy(pbBmpData, &bmHeader, sizeof(BITMAPFILEHEADER));
	memcpy(pbBmpData+sizeof(BITMAPFILEHEADER), &bmInfo, sizeof(BITMAPINFOHEADER));

	memcpy_s((BYTE*)(pbBmpData+54), dwBmpSize, buffer, dwBmpSize);

	FILE * f;
	errno_t err;
	if( (err = fopen_s(&f, szPath, "wb") ) == 0)
	{
		fwrite(pbBmpData, 1, dwBmpSize, f);
		fclose(f);
	}
	return TRUE;
}

WCHAR* CCommonFunc::GetDocumentPathW()
{
	SHGetSpecialFolderPath( NULL, g_wszDocumentPath, CSIDL_MYDOCUMENTS, FALSE );

	return g_wszDocumentPath;
}

CHAR* CCommonFunc::GetDocumentPathA()
{
	strcpy_s(g_szDocumentPath, WCharToChar(GetDocumentPathW()));
	return g_szDocumentPath;
}

#pragma comment(lib, "version.lib")
CString CCommonFunc::GetVersionInfo(HMODULE hLib, CString csEntry)
{
	CString csRet;

	if (hLib == NULL)
		hLib = AfxGetResourceHandle();

	HRSRC hVersion = FindResource( hLib, MAKEINTRESOURCE(VS_VERSION_INFO), RT_VERSION );
	if (hVersion != NULL)
	{
		HGLOBAL hGlobal = LoadResource( hLib, hVersion );
		if ( hGlobal != NULL)
		{

			LPVOID versionInfo = LockResource(hGlobal);
			if (versionInfo != NULL)
			{
				DWORD vLen,langD;
				BOOL retVal;

				LPVOID retbuf=NULL;
#ifdef _UNICODE
				static TCHAR fileEntry[256];
				wsprintf(fileEntry,_T("\\VarFileInfo\\Translation"));
#else
				static char fileEntry[256];
				sprintf(fileEntry,_T("\\VarFileInfo\\Translation"));
#endif


				retVal = VerQueryValue(versionInfo,fileEntry,&retbuf,(UINT *)&vLen);
				if (retVal && vLen==4)
				{
					memcpy(&langD,retbuf,4);
#ifdef _UNICODE
					wsprintf(fileEntry, _T("\\StringFileInfo\\%02X%02X%02X%02X\\%s"),
						(langD & 0xff00)>>8,langD & 0xff,(langD & 0xff000000)>>24,
						(langD & 0xff0000)>>16, csEntry);
#else
					sprintf(fileEntry, _T("\\StringFileInfo\\%02X%02X%02X%02X\\%s"),
						(langD & 0xff00)>>8,langD & 0xff,(langD & 0xff000000)>>24,
						(langD & 0xff0000)>>16, csEntry);
#endif
				}
				else
#ifdef _UNICODE
					wsprintf(fileEntry, L"\\StringFileInfo\\%04X04B0\\%s", GetUserDefaultLangID(), csEntry);
#else
					sprintf(fileEntry, "\\StringFileInfo\\%04X04B0\\%s", GetUserDefaultLangID(), (LPCTSTR)csEntry);
#endif

				if (VerQueryValue(versionInfo,fileEntry,&retbuf,(UINT *)&vLen))
#ifdef _UNICODE
					csRet = (TCHAR*)retbuf;
#else
					csRet = (char*)retbuf;
#endif
			}
		}

		UnlockResource( hGlobal );
		FreeResource( hGlobal );
	}

	return csRet;
}

CString CCommonFunc::FormatVersion(CString cs)
{
	CString csRet;
	if (!cs.IsEmpty())
	{
		cs.TrimRight();
		int iPos = cs.Find(',');
		if (iPos == -1)
			return L"";
		cs.TrimLeft();
		cs.TrimRight();
		csRet.Format(L"%s.", cs.Left(iPos));

		while (1)
		{
			cs = cs.Mid(iPos + 1);
			cs.TrimLeft();
			iPos = cs.Find(',');
			if (iPos == -1)
			{
				csRet +=cs;
				break;
			}
			csRet += cs.Left(iPos);
		}
	}

	return csRet;
}

BOOL CCommonFunc::KillProcess(CString sExeName)
{
    sExeName.MakeUpper();
    HANDLE hSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, 0 );
    if ( (int)hSnapshot != -1 )
    {
        PROCESSENTRY32 pe32 ;
        pe32.dwSize=sizeof(PROCESSENTRY32);
        BOOL bContinue ;
        CString strProcessName; 
        if ( Process32First ( hSnapshot, &pe32 ) )
        {
            do
            {
                strProcessName = pe32.szExeFile; //strProcessName이 프로세스 이름;
                strProcessName.MakeUpper();
                if( ( strProcessName.Find(sExeName,0) != -1 ) )
                {
                    HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID );
                    if( hProcess )
                    {
                        DWORD       dwExitCode;
                        GetExitCodeProcess( hProcess, &dwExitCode);
                        TerminateProcess( hProcess, dwExitCode); 
                        CloseHandle(hProcess);
                        CloseHandle( hSnapshot );
                        return TRUE;
                    } 
                    return FALSE;
                } 
                bContinue = Process32Next ( hSnapshot, &pe32 ); 
            } while ( bContinue );
        } 
        CloseHandle( hSnapshot );
    }
    return FALSE;
}

BOOL CCommonFunc::KillProcessAll(CString sExeName)
{
    sExeName.MakeUpper();
    HANDLE hSnapshot = CreateToolhelp32Snapshot ( TH32CS_SNAPPROCESS, 0 );
    if ( (int)hSnapshot != -1 )
    {
        PROCESSENTRY32 pe32 ;
        pe32.dwSize=sizeof(PROCESSENTRY32);
        BOOL bContinue ;
        CString strProcessName; 
        if ( Process32First ( hSnapshot, &pe32 ) )
        {
            do
            {
                strProcessName = pe32.szExeFile; //strProcessName이 프로세스 이름;
                strProcessName.MakeUpper();
                if( ( strProcessName.Find(sExeName,0) != -1 ) )
                {
                    HANDLE hProcess = OpenProcess( PROCESS_ALL_ACCESS, 0, pe32.th32ProcessID );
                    if( hProcess )
                    {
                        DWORD dwExitCode;
                        GetExitCodeProcess( hProcess, &dwExitCode);
                        TerminateProcess( hProcess, dwExitCode); 
                        CloseHandle(hProcess);
                    } 
                } 
                bContinue = Process32Next ( hSnapshot, &pe32 ); 
            } while ( bContinue );
        } 
        CloseHandle( hSnapshot );
    }
    return FALSE;
}

BOOL CCommonFunc::ShellExecuteProgram(WCHAR* szPath, WCHAR* args)
{
	ShellExecute(NULL, L"open", szPath, args, NULL, SW_SHOW);
	return TRUE;
}

BOOL CCommonFunc::ExecuteProgram(WCHAR* szPath)
{
	ShellExecute(NULL, L"open", szPath, NULL, NULL, SW_SHOW);
	return TRUE;

	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	::ZeroMemory(&si, sizeof(STARTUPINFO));
	::ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(STARTUPINFO);
	si.lpDesktop = L"winsta0\\default";
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = SW_SHOW;
	HANDLE hToken = NULL;
	HANDLE hProcess = NULL;
	// 현재 프로세스의 핸들로 토큰을 얻는다.
	hProcess = ::OpenProcess(PROCESS_QUERY_INFORMATION, TRUE, ::GetCurrentProcessId());
	BOOL bResult=::OpenProcessToken(hProcess, TOKEN_ALL_ACCESS, &hToken); // 프로세스 핸들로 프로세스의 보안 정보를 가지고 있는 토큰을 얻어내는 함수
	TCHAR* szCmdline=_tcsdup(szPath);
	// 얻어진 토큰을 가지고 프로세스를 생성한다.
	if(CreateProcessAsUser(
						hToken,
						NULL, // No module name (use line)
						szCmdline,
						NULL,
						NULL,
						TRUE,
						CREATE_NEW_CONSOLE | CREATE_NEW_PROCESS_GROUP,
						NULL,
						NULL,
						#ifdef UNICODE
						reinterpret_cast<LPSTARTUPINFOW>(&si), 
						#else
						(LPSTARTUPINFO)&si,
						#endif
						&pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
	}
	free(szCmdline);

	return TRUE;
}

BOOL CCommonFunc::ProgramCreateProcess(const WCHAR *pszTitle, const WCHAR *pszCommand, BOOL bShow, WCHAR* pszArg)
{
	long lErrorCode = -1;
	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter

	STARTUPINFO StartInfo;
	memset(&StartInfo, 0, sizeof(StartInfo));

	StartInfo.cb      = sizeof(StartInfo);
	StartInfo.lpTitle = (pszTitle) ? (WCHAR *)pszTitle : (WCHAR *)pszCommand;

	StartInfo.wShowWindow = bShow;
	StartInfo.dwFlags    |= STARTF_USESHOWWINDOW;

	WCHAR szCommand[2048];
	wsprintf(szCommand, L"\"%s\" %s", pszCommand, pszArg);

	if (CreateProcess(0, (WCHAR *)szCommand, 0, 0, TRUE, CREATE_NEW_PROCESS_GROUP, 0, 0, &StartInfo, &ProcessInfo))
		lErrorCode = 0;
	else lErrorCode = GetLastError();

	return (lErrorCode == 0);
}

BOOL CCommonFunc::ProgramCreateProcessA(const CHAR *pszTitle, const CHAR *pszCommand, BOOL bShow, CHAR* pszArg)
{
	long lErrorCode = -1;
	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter

	STARTUPINFOA StartInfo;
	memset(&StartInfo, 0, sizeof(StartInfo));

	StartInfo.cb      = sizeof(StartInfo);
	StartInfo.lpTitle = (pszTitle) ? (CHAR *)pszTitle : (CHAR *)pszCommand;

	StartInfo.wShowWindow = bShow;
	StartInfo.dwFlags    |= STARTF_USESHOWWINDOW;

	CHAR szCommand[2048];
	sprintf(szCommand, "\"%s\" %s", pszCommand, pszArg);

	if (CreateProcessA(0, (CHAR *)szCommand, 0, 0, TRUE, CREATE_NEW_PROCESS_GROUP, 0, 0, &StartInfo, &ProcessInfo))
		lErrorCode = 0;
	else lErrorCode = GetLastError();

	return (lErrorCode == 0);
}

void CCommonFunc::GetMyLocalIP(UINT & nIP)
{
	CStringA ip = "";
	char szHostName[256];
	PHOSTENT pHostInfo;

	if( gethostname(szHostName,sizeof(szHostName)) ==0)
	{
		if((pHostInfo = gethostbyname(szHostName)) != NULL)
		{
			ip = inet_ntoa(*(struct in_addr *)*pHostInfo->h_addr_list);
		}
	}
	nIP = ChangeIPAddressToINT(ip.GetBuffer(0));
	ip.ReleaseBuffer();
}

void CCommonFunc::GetMyLocalIP(CString & sIP)
{
	CStringA ip = "";
	char szHostName[256];
	PHOSTENT pHostInfo;

	if( gethostname(szHostName,sizeof(szHostName)) ==0)
	{
		if((pHostInfo = gethostbyname(szHostName)) != NULL)
		{
			ip = inet_ntoa(*(struct in_addr *)*pHostInfo->h_addr_list);
		}
	}
	sIP = CharToWCHAR(ip.GetBuffer(0));
	ip.ReleaseBuffer();
}

UINT CCommonFunc::ChangeIPAddressToINT(CHAR* szIP)
{
	return (UINT)inet_addr(szIP);
}

VOID CCommonFunc::ChangeIPAddressToString(UINT in, CHAR* szIP)
{
	char buf[16] = "000.000.000.000";  // NULL 포함
	char *p = (char *) &in;
	int pos = 0, nValue = 0;
	if(*p < 0) nValue = 256 + *p; else nValue = *p;
	pos = pos + sprintf(&buf[pos], "%d.", nValue); ++p;
	if(*p < 0) nValue = 256 + *p; else nValue = *p;
	pos = pos + sprintf(&buf[pos], "%d.", nValue); ++p;
	if(*p < 0) nValue = 256 + *p; else nValue = *p;
	pos = pos + sprintf(&buf[pos], "%d.", nValue); ++p;
	if(*p < 0) nValue = 256 + *p; else nValue = *p;
	pos = pos + sprintf(&buf[pos], "%d", nValue);

	if(buf)	strcpy_s(szIP, 16, buf);
}

HWND CCommonFunc::GetFullScreenWindow()
{
	HWND activeWnd = GetForegroundWindow();
	if(!activeWnd)
		return NULL;
	activeWnd = GetAncestor(activeWnd, GA_ROOT);
	if(!activeWnd)
		return NULL;
	TCHAR className[MAX_PATH] = {0};
	if(!GetClassName(activeWnd, className, MAX_PATH))
		return NULL;
	if(_tcsicmp(className, _T("WorkerW")) == 0)
		return NULL;
	if(_tcsicmp(className, _T("ProgMan")) == 0)
		return NULL;
	HWND desktopWnd = GetDesktopWindow();
	if(!desktopWnd)
		return NULL;
	RECT desktop;
	if(!GetWindowRect(desktopWnd, &desktop))
		return NULL;
	RECT client;
	if(!GetClientRect(activeWnd, &client))
		return NULL;
	SIZE clientSize = { client.right - client.left, client.bottom - client.top };
	SIZE desktopSize = { desktop.right - desktop.left, desktop.bottom - desktop.top };
	if(clientSize.cx < desktopSize.cx || clientSize.cy < desktopSize.cy)
		return NULL;
	return activeWnd;
}

// 1. 한글모드로 전환
void CCommonFunc::SetHanMode(HWND hWnd)
{
	HIMC hIMC = ImmGetContext(hWnd);
	DWORD dwConv, dwSent;
	DWORD dwTemp;

	ImmGetConversionStatus(hIMC,&dwConv,&dwSent);

	dwTemp = dwConv & ~IME_CMODE_LANGUAGE;
	dwTemp |= IME_CMODE_NATIVE;
	dwConv = dwTemp;
	ImmSetConversionStatus(hIMC,dwConv,dwSent);
	ImmReleaseContext(hWnd,hIMC);
}

// 2. 영문모드로 전환
void CCommonFunc::SetEngMode(HWND hWnd)
{
	HIMC hIMC = ImmGetContext(hWnd);
	DWORD dwConv, dwSent;
	DWORD dwTemp;

	ImmGetConversionStatus(hIMC,&dwConv,&dwSent);

	dwTemp = dwConv & ~IME_CMODE_LANGUAGE;
	dwTemp |= IME_CMODE_ALPHANUMERIC;
	dwConv = dwTemp;
	ImmSetConversionStatus(hIMC,dwConv,dwSent);

	ImmReleaseContext(hWnd,hIMC);
}

static BOOL sockinitialize(void)
{
    WSADATA data;
    if (WSAStartup (MAKEWORD(1, 1), &data) != 0)
    {
		return FALSE;
    }
	return TRUE;
}

static void sockuninitialise (void)
{
    WSACleanup ();
}

// URL 에서 IP 주소 가져오기
INT CCommonFunc::GetIPFromUrl(char* szUrl, INT & nCount, UINT * nIP)
{
	//sockinitialize();

	struct hostent *he;
	he = gethostbyname (szUrl);
	if (he == NULL)
    {
		//sockuninitialise ();
        switch (h_errno)
        {
            case HOST_NOT_FOUND:
                return HOST_NOT_FOUND;
            case NO_ADDRESS:
                return NO_ADDRESS;
            case NO_RECOVERY:
                return NO_RECOVERY;
            case TRY_AGAIN:
                return TRY_AGAIN;
			default:
				return -1;
        }
    }
	nCount = 0;
	for(INT nIndex = 0; nIndex < 4; nIndex++)
	{
		if(!he->h_addr_list[nIndex]) break;
		++nCount;

		nIP[nIndex] = *((DWORD*)he->h_addr_list[nIndex]);
	}

	//sockuninitialise ();
	return 0;
}

ULONG CCommonFunc::ProcIDFromWnd(HWND hwnd) // 윈도우 핸들로 프로세스 아이디 얻기   
{   
	ULONG idProc;   
	GetWindowThreadProcessId( hwnd, &idProc );   
	return idProc;   
}

HWND CCommonFunc::GetWinHandle(ULONG pid) // 프로세스 아이디로 윈도우 핸들 얻기   
{   
	HWND tempHwnd = FindWindow(NULL,NULL); // 최상위 윈도우 핸들 찾기   

	ULONG idProc;
	while(tempHwnd != NULL)
	{
		if(::GetParent(tempHwnd) == NULL)
		{
			GetWindowThreadProcessId(tempHwnd, &idProc);
			if(pid == idProc)
				return tempHwnd;
		}
		tempHwnd = GetWindow(tempHwnd, GW_HWNDNEXT);
	}
	return NULL;   
} 

time_t CCommonFunc::MakeTimeT(int YYYY, int MM, int DD, int hh, int mi, int ss)
{
	struct tm st_tm;

	st_tm.tm_year = YYYY - 1900;
	st_tm.tm_mon =  MM - 1;
	st_tm.tm_mday = DD;
	st_tm.tm_hour = hh;
	st_tm.tm_min =  mi;
	st_tm.tm_sec =  ss;

	return mktime( &st_tm );
}

UINT CCommonFunc::GetAtoUI(char* pValue)
{
	char ** enddl=NULL;
	UINT nValue = strtoul(pValue, enddl, 10);
	return nValue;
}

CSize CCommonFunc::GetTextExtent(CDC* _pDC, CString sText)
{
	CSize size = _pDC->GetTextExtent(sText);
	return size;
}

#include <gdiPlus.h> //header 포함
#pragma comment(lib, "gdiplus")
using namespace Gdiplus; //네임스페이스 설정

static int GetEncoderClsid(const WCHAR *format, CLSID *pClsid)  
{  
    UINT num = 0;  
    UINT size = 0;  
    ImageCodecInfo * pImageCodecInfo = NULL;  
    GetImageEncodersSize(&num, &size);  
    if(size == 0)  
        return -1;  
  
    pImageCodecInfo = (ImageCodecInfo *)(malloc(size));  
    if(pImageCodecInfo == NULL)  
        return -1;  
  
    GetImageEncoders(num, size, pImageCodecInfo);  
    for(UINT j = 0 ; j < num; ++j)  
    {  
        if(wcscmp(pImageCodecInfo[j].MimeType, format) == 0)  
        {  
            *pClsid = pImageCodecInfo[j].Clsid;  
            free(pImageCodecInfo);  
            return j;  
        }  
  
    }
    free(pImageCodecInfo);  
    return -1;  
}  

BOOL CCommonFunc::SaveDCtoImage(CBitmap & bitmap, INT nWidth, INT nHeight, CString sPath)
{
	HPALETTE hpal;  
	hpal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);  

	Bitmap image((HBITMAP)bitmap, hpal);

	CLSID jpegClsid;  
	GetEncoderClsid(L"image/jpeg", &jpegClsid);  
	image.Save(sPath, &jpegClsid, NULL);  

	return TRUE;
}

#include <Windows.h>
#pragma comment(lib,"version.lib")
BOOL CCommonFunc::GetFileVersion(CString sFilePath, CString & sVersion)
{
	INT nInfoSize = GetFileVersionInfoSize(sFilePath, 0);
	if(nInfoSize == 0)
		return FALSE;

	BYTE buffer[1024];
	if(GetFileVersionInfo(sFilePath, 0, nInfoSize, buffer) != 0)
	{
		 VS_FIXEDFILEINFO* pFineInfo = NULL;
		UINT bufLen = 0;
		// buffer로 부터 VS_FIXEDFILEINFO 정보를 가져옵니다.
		if(VerQueryValue(buffer, _T("\\"), (LPVOID*)&pFineInfo, &bufLen) != 0)
		{    
			WORD majorVer, minorVer, buildNum, revisionNum;
			majorVer = HIWORD(pFineInfo->dwFileVersionMS);
			minorVer = LOWORD(pFineInfo->dwFileVersionMS);
			buildNum = HIWORD(pFineInfo->dwFileVersionLS);
			revisionNum = LOWORD(pFineInfo->dwFileVersionLS);

			sVersion.Format(_T("%d.%d.%d.%d"), majorVer, minorVer, buildNum, revisionNum);

			return TRUE;
		}
	}
	return FALSE;
}

BOOL CCommonFunc::RegistryDisableHighDpiAware(WCHAR* pPath/*=NULL*/)
{
	WCHAR szPath[2048];
	if(!pPath)
		GetModuleFileName(NULL, szPath, 2048);
	else wcscpy_s(szPath, pPath);

	HKEY key;
// 	wstring featuresPath(L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\\");
// 	wstring path(featuresPath);
// 
// 	LONG nError = RegCreateKeyEx(HKEY_CURRENT_USER, path.c_str(), 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &key, NULL);

	CString featuresPath(L"Software\\Microsoft\\Windows NT\\CurrentVersion\\AppCompatFlags\\Layers\\");
	CString path(featuresPath);

	LONG nError = RegCreateKeyEx(HKEY_CURRENT_USER, path, 0, NULL, REG_OPTION_VOLATILE, KEY_WRITE, NULL, &key, NULL);
	if (nError != ERROR_SUCCESS) {
		return FALSE;
	} else {
		WCHAR* szValue = L"~ HIGHDPIAWARE";
		nError = RegSetValueExW(key, szPath, 0, REG_SZ, (const BYTE*) szValue, sizeof(WCHAR)*wcslen(szValue));
		if (nError != ERROR_SUCCESS) {
			RegCloseKey(key);
			return FALSE;
		}

		nError = RegCloseKey(key);
		if (nError != ERROR_SUCCESS) {
			return FALSE;
		}
	}
	return TRUE;
}

CString CCommonFunc::GetProgramPath()
{
	CString strPath;
	TCHAR szPath[1024];
	GetModuleFileName(NULL, szPath, 1024);
	strPath.Format(_T("%s"), szPath);
	return strPath;
}

CString CCommonFunc::GetProgramDir()
{
	CString strPath;
	TCHAR szPath[1024];
	GetModuleFileName(NULL, szPath, 1024);
	strPath.Format(_T("%s"), szPath);
	strPath = strPath.Left(strPath.ReverseFind('\\'));
	return strPath;
}

CString CCommonFunc::GetFileNameOnly(CString strPath)
{
	CString strFileName;
	strFileName = strPath.Right(strPath.GetLength() - strPath.ReverseFind('\\') - 1);
	return strFileName;
}

CString CCommonFunc::GetProgramVersion()
{
	CString strProgramVersion = _T("");
	CString strDate, strTime;

	strDate.Format(_T("%s"), CCommonFunc::CharToTCHAR(__DATE__));
	strTime.Format(_T("%s"), CCommonFunc::CharToTCHAR(__TIME__));

	strProgramVersion.Format(_T("%s %s"), strDate, strTime);

	return strProgramVersion;
}