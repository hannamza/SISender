// ProcWrapper.cpp: implementation of the CProcWrapper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProcWrapper.h"
#include <TlHelp32.h>
#include <Winternl.h>


typedef NTSTATUS(NTAPI *_NtQueryInformationProcess)(
	HANDLE ProcessHandle,
	DWORD ProcessInformationClass,
	PVOID ProcessInformation,
	DWORD ProcessInformationLength,
	PDWORD ReturnLength
	);


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProcWrapper::CProcWrapper()
{	
}

CProcWrapper::~CProcWrapper()
{

}

//---------------------------------------------------------
// Run a synchronized other command line EXE. Returns only 
// after this exits. The process is runned as a console window.
// Returns Values : TRUE if the process was created
//                  FALSE if not.
// see *nRetValue for the LastError number
BOOL CProcWrapper::RunProcessAndWait(const TCHAR *sCmdLine, const TCHAR *sArg, int *nRetValue)
{
	 int nRetWait;
	 int nError;

	 // That means wait 30 s before returning an error
	 // You can change it to the value you need.
	 // If you want to wait for ever just use 'dwTimeout = INFINITE'>
	 DWORD dwTimeout = 1000 *30; // 30초
	 
	 STARTUPINFO stInfo;
	 PROCESS_INFORMATION prInfo;
	 BOOL bResult;
	 ZeroMemory( &stInfo, sizeof(stInfo) );
	 stInfo.cb = sizeof(stInfo);
	 stInfo.dwFlags=STARTF_USESHOWWINDOW;
	 stInfo.wShowWindow=SW_HIDE;

	 bResult = CreateProcess(sCmdLine, 
							 (TCHAR *)sArg, 
							 NULL, 
							 NULL, 
							 TRUE,
							 CREATE_NEW_CONSOLE 
							 | NORMAL_PRIORITY_CLASS,
							 NULL,
							 NULL,
							 &stInfo, 
							 &prInfo);

	 nError = GetLastError();
 
	 if(!bResult)
	 {
		 CString err;
		 CString cmd = sCmdLine;
		 err.Format(_T("실행화일 : %s 를 실행할 수 없습니다 !"), cmd);
		 return FALSE;
	 }

	 nRetWait =  WaitForSingleObject(prInfo.hProcess, dwTimeout);
	 GetExitCodeProcess(prInfo.hProcess, (unsigned long *)nRetValue);
 
	 CloseHandle(prInfo.hThread); 
	 CloseHandle(prInfo.hProcess); 

	 if(nRetWait == WAIT_TIMEOUT) 
	 {
		 CString err;
		 CString cmd = sCmdLine;
		 err.Format(_T("실행화일 : %s 를 실행하는데 TIMEOUT 되었습니다 !"), cmd);
		 return FALSE;
	 }

	 return TRUE;
}

BOOL CProcWrapper::RunProcessAndWait2(const TCHAR *sCmdLine, 
									 const TCHAR *sArg, 
									 int *nRetValue)
{
	int nRetWait;
	int nError;

	// That means wait 30 s before returning an error
	// You can change it to the value you need.
	// If you want to wait for ever just use 'dwTimeout = INFINITE'>
	DWORD dwTimeout = 1000 *30;  

	STARTUPINFO stInfo;
	PROCESS_INFORMATION prInfo;
	BOOL bResult;
	ZeroMemory( &stInfo, sizeof(stInfo) );
	stInfo.cb = sizeof(stInfo);
	stInfo.dwFlags=STARTF_USESHOWWINDOW;
	stInfo.wShowWindow=SW_HIDE;

	bResult = CreateProcess(sCmdLine, 
		(TCHAR *)sArg, 	
		NULL, 
		NULL, 
		TRUE,
		CREATE_NEW_CONSOLE 
		| NORMAL_PRIORITY_CLASS,
		NULL,
		sArg,
		&stInfo, 
		&prInfo);

	nError = GetLastError();

	if(!bResult)
	{
		CString err;
		CString cmd = sCmdLine;
		err.Format(_T("실행화일 : %s 를 실행할 수 없습니다 !"), cmd);
		return FALSE;
	}

	nRetWait =  WaitForSingleObject(prInfo.hProcess,dwTimeout);
	GetExitCodeProcess(prInfo.hProcess, (unsigned long *)nRetValue);
	CloseHandle(prInfo.hThread); 
	CloseHandle(prInfo.hProcess); 

	if(nRetWait == WAIT_TIMEOUT) 
	{
		CString err;
		CString cmd = sCmdLine;
		err.Format(_T("실행화일 : %s 를 실행하는데 TIMEOUT 되었습니다 !"), cmd);
		return FALSE;
	}

	return TRUE;
}


//---------------------------------------------------------
// This function call a command line process.
// Returns Values : TRUE if the process was created
//                  FALSE if not.
// see *nRetValue for the LastError number
int CProcWrapper::RunProcessAndForget(const TCHAR *sCmdLine,
                         const TCHAR *sArg,
                         int *nRetValue)
{
	 int nError;
	 STARTUPINFO stInfo;
	 PROCESS_INFORMATION prInfo;
	 BOOL bResult;
	 ZeroMemory( &stInfo, sizeof(stInfo) );
	 stInfo.cb = sizeof(stInfo);
	 stInfo.dwFlags=STARTF_USESHOWWINDOW;
	 stInfo.wShowWindow=SW_SHOW;
		
	 bResult = CreateProcess(sCmdLine, 
							 (TCHAR *)sArg, 
							 NULL, 
							 NULL, 
							 TRUE,
							 CREATE_NEW_CONSOLE 
							 | NORMAL_PRIORITY_CLASS ,
							 NULL,
							 NULL,
							 &stInfo, 
							 &prInfo);

	 *nRetValue = nError = GetLastError();
 
	 // Don't write these two lines if you need
	 CloseHandle(prInfo.hThread); 

	 // to use these handles
	 CloseHandle(prInfo.hProcess);
 
	 if(!bResult)
	 {
		 CString err;
		 CString cmd = sCmdLine;
		 err.Format(_T("실행화일 : %s 를 실행할 수 없습니다 !"), cmd);
		 return 0;
	 }

	 return prInfo.dwProcessId;	 // return PID
}

// DOS, MDI Program 때문에...
BOOL CProcWrapper::RunProcessAndForget2(const TCHAR *sCmdLine,
                         const TCHAR *sArg,
                         int *nRetValue)
{
	 int nError;
	 STARTUPINFO stInfo;
	 PROCESS_INFORMATION prInfo;
	 BOOL bResult;
	 ZeroMemory( &stInfo, sizeof(stInfo) );
	 stInfo.cb = sizeof(stInfo);
	 stInfo.dwFlags=STARTF_USESHOWWINDOW;
	 stInfo.wShowWindow=SW_HIDE; //

	 bResult = CreateProcess(sCmdLine, 
							 (TCHAR *)sArg, 							
							 NULL, 
							 NULL, 
							 TRUE,
							 CREATE_NEW_CONSOLE 
							 | NORMAL_PRIORITY_CLASS ,
							 NULL,
							 NULL,
							 &stInfo, 
							 &prInfo);

	 *nRetValue = nError = GetLastError();
 
	 // Don't write these two lines if you need
	 CloseHandle(prInfo.hThread); 

	 // to use these handles
	 CloseHandle(prInfo.hProcess);
 
	 if(!bResult)
	 {
		 CString err;
		 CString cmd = sCmdLine;
		 err.Format(_T("실행화일 : %s 를 실행할 수 없습니다 !"), cmd);		 
		 return FALSE;
	 }

	 return TRUE;	
}

BOOL CProcWrapper::IsFindProc(int nPid, CString sPName)
{
	CString sTmp = sPName;
	CString sRunnigPName;
	sRunnigPName = GetProcCommandLine(nPid);

	sPName.Replace(_T(" "), _T(""));
	sRunnigPName.Replace(_T(" "), _T(""));

//	sPName.MakeLower();
//	sRunnigPName.MakeLower();

	if (!sRunnigPName.CompareNoCase(sPName))
	{
		//TRACE(">>>>> [Find Process : %s]\n", sTmp);
		return TRUE;
	}
	
	return FALSE;
}

int CProcWrapper::GetProcIdNoCmdLine(const TCHAR *in)
{
	CString proc = in;
	CString runProc;
	int proc_cnt = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcess, &pe32))
	{
		do
		{
			if (proc.Find(pe32.szExeFile) != -1)
			{
				proc_cnt = pe32.th32ProcessID;
				break;
			}
		} while (Process32Next(hProcess, &pe32));
	}

	CloseHandle(hProcess);
	return proc_cnt;
}

int CProcWrapper::GetProcId(const TCHAR *in)
{
	CString proc = in;
	CString runProc;
	int proc_cnt = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcess, &pe32))
	{
		do
		{
			if (IsFindProc(pe32.th32ProcessID, proc))
			{
				proc_cnt = pe32.th32ProcessID;
				break;
			}
		} while (Process32Next(hProcess, &pe32));
	}

	CloseHandle(hProcess);
	return proc_cnt;
	}

int CProcWrapper::GetProcCnt(const TCHAR *in)
{
	CString proc = in;
	CString runProc;
	int proc_cnt = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = {0};	

	hProcess = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( Process32First( hProcess, &pe32 ) )
	{
		do
		{
			if (IsFindProc(pe32.th32ProcessID, proc))
				++proc_cnt;
		} 
		while ( Process32Next( hProcess, &pe32 ) );
	}

	CloseHandle(hProcess);
	return proc_cnt;
}

BOOL CProcWrapper::ForceKillProcess(const TCHAR *in)
{
	CString proc = in;	
	BOOL res = TRUE;
	DWORD parentId = 0;
	DWORD childId = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = {0};	

	hProcess = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( Process32First( hProcess, &pe32 ) )
	{
		do
		{
			if( proc.Find(pe32.szExeFile) != -1 )
			{				
				childId = pe32.th32ProcessID;

				HANDLE ps = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, 
					FALSE, childId);
				res = TerminateProcess(ps, 0);
				CloseHandle(ps);

				if (!res)
					break;
			}			
		} 
		while ( Process32Next( hProcess, &pe32 ) );
	}

	CloseHandle(hProcess);

	return res;
}

BOOL CALLBACK EnumWindowsCloseProc(HWND hwnd, LPARAM lParam)
{
	DWORD wndPid;
	TCHAR Title[100];

	GetWindowThreadProcessId(hwnd, &wndPid);
	memset(Title, 0, 100);
	::GetWindowText(hwnd, Title, 100);
	if(wndPid == (DWORD)lParam && wcslen(Title) != 0)
	{
		//  Please kindly close this process
		::SendMessage(hwnd, WM_CLOSE, 0, 0);
		return false;
	}

	// Keep enumerating
	return true;
}

void CProcWrapper::SoftKillProcess(const TCHAR *in)
{
	CString proc = in;	
	DWORD processPid = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = {0};

	hProcess = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( Process32First( hProcess, &pe32 ))
	{
		do
		{
			if( proc.Find(pe32.szExeFile) != -1 )
			{				
				processPid = pe32.th32ProcessID;
				HANDLE ps = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, processPid);
				EnumWindows(EnumWindowsCloseProc, processPid);
				CloseHandle(ps) ;
			}
		} 
		while ( Process32Next( hProcess, &pe32 ) );
	}

	CloseHandle(hProcess);
}

void CProcWrapper::SoftKillProcess(DWORD _dwProcessID)
{
#if 0
	DWORD processPid = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = {0};

	hProcess = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( Process32First( hProcess, &pe32 ) )
	{
		do
		{
		//	if( proc.Find(pe32.szExeFile) != -1 )
			if (_dwProcessID == pe32.th32ProcessID)
			{				
				processPid = pe32.th32ProcessID;
				HANDLE ps = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, processPid);
				EnumWindows(EnumWindowsCloseProc, processPid);
			//	::SendMessage(ps, WM_CLOSE, 0, 0);
				CloseHandle(ps) ;
			}
		} 
		while ( Process32Next( hProcess, &pe32 ) );
	}

	CloseHandle(hProcess);
#else
	EnumWindows(EnumWindowsCloseProc, _dwProcessID);
#endif
}

BOOL CALLBACK EnumWindowsShowProc(HWND hwnd, LPARAM lParam)
{

	DWORD wndPid;
	TCHAR Title[100];

	GetWindowThreadProcessId(hwnd, &wndPid);
	memset(Title, 0, 100);
	::GetWindowText(hwnd, Title, 100);
	if(wndPid == (DWORD)lParam && wcslen(Title) != 0)
	{
		//  Please kindly close this process
#if 0
		::SendMessage(hwnd, WM_SHOWWINDOW, 0, 0);
#else
		::PostMessage(hwnd, WM_SYSCOMMAND, SC_RESTORE, NULL);
		::PostMessage(hwnd, WM_SHOWWINDOW, TRUE, SW_OTHERUNZOOM);
#endif
		return false;
	}

	// Keep enumerating

	return TRUE;
}
 
void CProcWrapper::ShowProcess(const TCHAR *in)
{
	CString proc = in;	
	DWORD processPid = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = {0};

	hProcess = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( Process32First( hProcess, &pe32 ) )
	{
		do
		{
			//if( proc.Find(pe32.szExeFile) != -1 )
			if (IsFindProc(pe32.th32ProcessID, proc))
			{				
				processPid = pe32.th32ProcessID;
				HANDLE ps = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, processPid);
				EnumWindows(EnumWindowsShowProc, processPid);
				CloseHandle(ps);
			}
		} 
		while ( Process32Next( hProcess, &pe32 ) );
	}

	CloseHandle(hProcess);
}

void CProcWrapper::ShowProcess(DWORD _dwProcessID)
{
#if 0
	CString proc = in;
	DWORD processPid = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcess, &pe32))
	{
		do
		{
			//if( proc.Find(pe32.szExeFile) != -1 )
			if (IsFindProc(pe32.th32ProcessID, proc))
			{
				processPid = pe32.th32ProcessID;
				HANDLE ps = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, processPid);
				EnumWindows(EnumWindowsShowProc, processPid);
				CloseHandle(ps);
			}
		} while (Process32Next(hProcess, &pe32));
	}

	CloseHandle(hProcess);
#else
	EnumWindows(EnumWindowsShowProc, _dwProcessID);
#endif
}

BOOL CALLBACK EnumWindowsHideProc(HWND hwnd, LPARAM lParam)
{
	DWORD wndPid;
	TCHAR Title[100];

	GetWindowThreadProcessId(hwnd, &wndPid);
	memset(Title, 0, 100);
	::GetWindowText(hwnd, Title, 100);
	if(wndPid == (DWORD)lParam && wcslen(Title) != 0)
	{
		//  Please kindly close this process
	//	::SendMessage(hwnd, WM_APPLICATION_HIDE, 0, 0);
#if 0
		::PostMessage(hwnd, WM_SHOWWINDOW, FALSE, SW_OTHERUNZOOM);
#else
		::PostMessage(hwnd, WM_SYSCOMMAND, SC_MINIMIZE, NULL);
		::PostMessage(hwnd, WM_SHOWWINDOW, FALSE, SW_OTHERUNZOOM);
#endif
		return false;
	}

	// Keep enumerating
	return TRUE;
}

void SetForegroundWindowForce(HWND hWnd)
{
	HWND hWndForeground = ::GetForegroundWindow();
	if (hWndForeground == hWnd) return;

	DWORD Strange = ::GetWindowThreadProcessId(hWndForeground, NULL);
	DWORD My = ::GetWindowThreadProcessId(hWnd, NULL);
	if (!::AttachThreadInput(Strange, My, TRUE))
	{
		ASSERT(0);
	}
	::SetForegroundWindow(hWnd);
	::BringWindowToTop(hWnd);
	::ShowWindow(hWnd, SW_SHOW);
// 	if (!::AttachThreadInput(My, Strange, FALSE))
// 	{
// 		ASSERT(0);
// 	}
}

BOOL CALLBACK EnumWindowsTopProc(HWND hwnd, LPARAM lParam)
{
	DWORD wndPid;
	TCHAR Title[100];

	GetWindowThreadProcessId(hwnd, &wndPid);
	memset(Title, 0, 100);
	::GetWindowText(hwnd, Title, 100);
	if (wndPid == (DWORD)lParam && wcslen(Title) != 0)
	{
		//  Please kindly close this process
		SetForegroundWindowForce(hwnd);
		return false;
	}

	// Keep enumerating
	return TRUE;
}

void CProcWrapper::HideProcess(const TCHAR *in)
{
	CString proc = in;	
	DWORD processPid = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = {0};

	hProcess = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
	pe32.dwSize = sizeof( PROCESSENTRY32 );
	if( Process32First( hProcess, &pe32 ) )
	{
		do
		{
			//if( proc.Find(pe32.szExeFile) != -1 )
			if (IsFindProc(pe32.th32ProcessID, proc))
			{
				processPid = pe32.th32ProcessID;
				HANDLE ps = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, processPid);
				EnumWindows(EnumWindowsHideProc, processPid);
				CloseHandle(ps);
			}
		} 
		while ( Process32Next( hProcess, &pe32 ) );
	}

	CloseHandle(hProcess);
}

void CProcWrapper::HideProcess(DWORD _dwProcessID)
{
#if 0
	CString proc = in;
	DWORD processPid = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcess, &pe32))
	{
		do
		{
			//if( proc.Find(pe32.szExeFile) != -1 )
			if (IsFindProc(pe32.th32ProcessID, proc))
			{
				processPid = pe32.th32ProcessID;
				HANDLE ps = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, processPid);
				EnumWindows(EnumWindowsHideProc, processPid);
				CloseHandle(ps);
			}
		} while (Process32Next(hProcess, &pe32));
	}

	CloseHandle(hProcess);
#else
	EnumWindows(EnumWindowsHideProc, _dwProcessID);
#endif
}

void CProcWrapper::TopProcess(const TCHAR *in)
{
	CString proc = in;
	DWORD processPid = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcess, &pe32))
	{
		do
		{
			//if (proc.Find(pe32.szExeFile) != -1)
			if (IsFindProc(pe32.th32ProcessID, proc))
			{
				processPid = pe32.th32ProcessID;
				HANDLE ps = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, processPid);
				EnumWindows(EnumWindowsTopProc, processPid);
				CloseHandle(ps);
			}
		} while (Process32Next(hProcess, &pe32));
	}

	CloseHandle(hProcess);
}

void CProcWrapper::TopProcess(DWORD _dwProcessID)
{
#if 0
	CString proc = in;
	DWORD processPid = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcess, &pe32))
	{
		do
		{
			//if (proc.Find(pe32.szExeFile) != -1)
			if (IsFindProc(pe32.th32ProcessID, proc))
			{
				processPid = pe32.th32ProcessID;
				HANDLE ps = OpenProcess(SYNCHRONIZE | PROCESS_TERMINATE, FALSE, processPid);
				EnumWindows(EnumWindowsTopProc, processPid);
				CloseHandle(ps);
			}
		} while (Process32Next(hProcess, &pe32));
	}

	CloseHandle(hProcess);
#else
	EnumWindows(EnumWindowsTopProc, _dwProcessID);
#endif
}

PVOID CProcWrapper::GetPebAddress(HANDLE ProcessHandle)
{
	_NtQueryInformationProcess NtQueryInformationProcess =
		(_NtQueryInformationProcess)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtQueryInformationProcess");

	PROCESS_BASIC_INFORMATION pbi;
	NtQueryInformationProcess(ProcessHandle, (PROCESSINFOCLASS)0, &pbi, sizeof(pbi), NULL);

	return pbi.PebBaseAddress;
}
 
CString CProcWrapper::GetProcCommandLine(int nPid)
{
	int pid;
	HANDLE processHandle;
	PVOID pebAddress;
	PVOID rtlUserProcParamsAddress;
	UNICODE_STRING commandLine;
	WCHAR *commandLineContents;
	CString sResult;

	pid = nPid;

	if ((processHandle = OpenProcess(
		PROCESS_QUERY_INFORMATION | /* required for NtQueryInformationProcess */
		PROCESS_VM_READ, /* required for ReadProcessMemory */
		FALSE, pid)) == 0)
	{
		//TRACE("Could not open process!\n");
		return NULL;
	}

	pebAddress = GetPebAddress(processHandle);

	/* get the address of ProcessParameters */
	if (!ReadProcessMemory(processHandle, (PCHAR)pebAddress + 0x10,
		&rtlUserProcParamsAddress, sizeof(PVOID), NULL))
	{
		//TRACE("Could not read the address of ProcessParameters!\n");
		CloseHandle(processHandle);
		return NULL;
	}

	/* read the CommandLine UNICODE_STRING structure */
	if (!ReadProcessMemory(processHandle, (PCHAR)rtlUserProcParamsAddress + 0x40,
		&commandLine, sizeof(commandLine), NULL))
	{
		TRACE("Could not read CommandLine! : error code : %d\n", GetLastError());
		CloseHandle(processHandle);
		return NULL;
	}

	/* allocate memory to hold the command line */
	commandLineContents = (WCHAR *)malloc(commandLine.Length);

	/* read the command line */
	if (!ReadProcessMemory(processHandle, commandLine.Buffer,
		commandLineContents, commandLine.Length, NULL))
	{
		//TRACE("Could not read the command line string!\n");
		CloseHandle(processHandle);
		free(commandLineContents);
		return NULL;
	}

	/* print it */
	/* the length specifier is in characters, but commandLine.Length is in bytes */
	/* a WCHAR is 2 bytes */

	sResult.Format(_T("%s\n"), CW2T( commandLineContents));
	sResult.Delete(commandLine.Length / 2, commandLine.Length); // 필요한 정보만 자른다.
	sResult.Replace(_T("\""), _T(""));
	TRACE("%s\n", sResult);

	CloseHandle(processHandle);
	free(commandLineContents);

	return sResult;
}

// JYS 16.01.06 중복 실행 오류 <ADD.S>
BOOL CProcWrapper::OpenProc(int nPid)
{
	HANDLE processHandle;
	CString sResult;

	if ((processHandle = OpenProcess(
		PROCESS_QUERY_INFORMATION | /* required for NtQueryInformationProcess */
		PROCESS_VM_READ, /* required for ReadProcessMemory */
		FALSE, nPid)) == 0)
	{
		//TRACE("Could not open process!\n");
		return FALSE;
	}

	CloseHandle(processHandle);

	return TRUE;
}
// JYS 16.01.06 중복 실행 오류 <ADD.E>

bool CProcWrapper::existProcess(int nPid)
{
	bool bFind = false;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcess, &pe32))
	{
		do
		{
			if (nPid == pe32.th32ProcessID) {
				bFind = true;
				break;
			}

		} while (Process32Next(hProcess, &pe32));
	}

	CloseHandle(hProcess);

	return bFind;
}

bool CProcWrapper::existProcess(const TCHAR *in)
{
	bool bFind = false;
	CString proc = in;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcess, &pe32))
	{
		do
		{
			if (proc.Find(pe32.szExeFile) != -1) {
				bFind = true;
				break;
			}

		} while (Process32Next(hProcess, &pe32));
	}

	CloseHandle(hProcess);

	return bFind;
}

int CProcWrapper::existProcess_pidReturn(const TCHAR *in)
{
	CString proc = in;
	int pid = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcess, &pe32))
	{
		do
		{
			if (proc.Find(pe32.szExeFile) != -1) {
				pid = pe32.th32ProcessID;
				break;
			}

		} while (Process32Next(hProcess, &pe32));
	}

	CloseHandle(hProcess);

	return pid;
}

// 중복 실행 검사 함수.
int CProcWrapper::existProcess_processCountReturn(const TCHAR *in)
{
	CString proc = in;
	int proc_cnt = 0;

	HANDLE   hProcess = NULL;
	PROCESSENTRY32 pe32 = { 0 };

	hProcess = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (Process32First(hProcess, &pe32))
	{
		do
		{
			if (proc.Find(pe32.szExeFile) != -1)
				proc_cnt++;

		} while (Process32Next(hProcess, &pe32));
	}

	CloseHandle(hProcess);

	return proc_cnt;
}

DWORD CProcWrapper::GetProcessIDByFileName(LPWSTR name)
{
	DWORD process_id_array[1024];
	DWORD bytes_returned;
	DWORD num_processes;
	HANDLE hProcess;
	WCHAR image_name[MAX_PATH] = { 0, };

	DWORD i;
	EnumProcesses(process_id_array, 1024 * sizeof(DWORD), &bytes_returned);
	num_processes = (bytes_returned / sizeof(DWORD));
	for (i = 0; i < num_processes; i++) {
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, process_id_array[i]);
		if (GetModuleBaseName(hProcess, 0, image_name, 256)) {
			if (!wcscmp(image_name, name)) {
				CloseHandle(hProcess);
				return process_id_array[i];
			}
		}
		CloseHandle(hProcess);
	}
	return 0;
}

BOOL CProcWrapper::GetProcessIDsByFileName(LPWSTR name, std::vector<DWORD>& vecProcessID)
{
	BOOL bRet = FALSE;

	DWORD process_id_array[1024];
	DWORD bytes_returned;
	DWORD num_processes;
	HANDLE hProcess;
	WCHAR image_name[MAX_PATH] = { 0, };

	DWORD i;
	EnumProcesses(process_id_array, 1024 * sizeof(DWORD), &bytes_returned);
	num_processes = (bytes_returned / sizeof(DWORD));
	for (i = 0; i < num_processes; i++) {
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, TRUE, process_id_array[i]);
		if (GetModuleBaseName(hProcess, 0, image_name, 256)) {
			if (!wcscmp(image_name, name)) {
				vecProcessID.push_back(process_id_array[i]);
				bRet = TRUE;
			}
		}
		CloseHandle(hProcess);
	}

	return bRet;
}