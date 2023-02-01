
#pragma once

#include <process.h>

#include <psapi.h>
#include <processthreadsapi.h>

enum _USER_MSG {
	WM_APPLICATION_SHOW = WM_USER + 100,
	WM_APPLICATION_HIDE,
};

class CProcWrapper : public TSingleton<CProcWrapper>  
{
public:
	CProcWrapper();
	virtual ~CProcWrapper();	

	int RunProcessAndForget(const TCHAR *sCmdLine, const TCHAR *sArg, int *nRetValue);
	BOOL RunProcessAndForget2(const TCHAR *sCmdLine, const TCHAR *sArg, int *nRetValue);
	
	BOOL RunProcessAndWait(const TCHAR *sCmdLine, const TCHAR *sArg, int *nRetValue);	
	BOOL RunProcessAndWait2(const TCHAR *sCmdLine, const TCHAR *sArg, int *nRetValue);

	int GetProcCnt(const TCHAR *in);
	int GetProcId(const TCHAR *in);
	int GetProcIdNoCmdLine(const TCHAR *in);
	BOOL ForceKillProcess(const TCHAR *in);

	void SoftKillProcess(const TCHAR *in);
	void SoftKillProcess(DWORD _dwProcessID);

	void ShowProcess(const TCHAR *in);
	void ShowProcess(DWORD _dwProcessID);

	void HideProcess(const TCHAR *in);
	void HideProcess(DWORD _dwProcessID);

	void TopProcess(const TCHAR *in);
	void TopProcess(DWORD _dwProcessID);

	PVOID GetPebAddress(HANDLE ProcessHandle);
	CString GetProcCommandLine(int nPid);
	BOOL IsFindProc(int nPid, CString sPName);
	BOOL OpenProc(int nPid); // JYS 16.01.06 중복 실행 오류 <ADD.L>

	bool existProcess(int nPid);
	bool existProcess(const TCHAR *in);
	int existProcess_pidReturn(const TCHAR *in);
	int existProcess_processCountReturn(const TCHAR *in);

	//Process ID 찾는 매서드, 프로세스 죽일 때는 무조건 이 매서드로 Process ID 찾아서 죽여야 함 (프로그램 이름으로는 잘 안됨)
	DWORD GetProcessIDByFileName(LPWSTR name);

	BOOL GetProcessIDsByFileName(LPWSTR name, std::vector<DWORD>& vecProcessID);
};