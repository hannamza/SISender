#pragma once
#include "stdafx.h"
#include "Log.h"
#include <windows.h>
#include <iostream>
#include <iomanip>
#include <Shlwapi.h>
#include <atltime.h>

using namespace std;

#define		LOG_BUFFER_CNT		1200
#define		LOG_TEXT_SIZE		512
namespace Log
{
	const int BUFFER_SIZE = 256;

	static bool s_Enable = true;
	CWnd* pMainWnd = NULL;

	HMODULE libModule;
	CRITICAL_SECTION logCS, logFile;
	FILE* file = NULL;
	int nTime = -1;
	char szLogtext[LOG_BUFFER_CNT][LOG_TEXT_SIZE];
	int nLogIndex = 0;

	char szDebugLog[10][2048];
	int nDebugIndex = 0;

	CString sLogFolderName = _T("");

	void SetMainPointer(CWnd* pWnd)
	{
		pMainWnd = pWnd;
	}

	void log_file_open()
	{
		EnterCriticalSection(&logFile);
		if (!file)
		{
			CString sLogFileName;
			CTime time = CTime::GetCurrentTime();
			sLogFileName.Format(_T("%s\\%d\\%d\\%d\\"), sLogFolderName, time.GetYear(), time.GetMonth(), time.GetDay());
			//sLogFileName.Format(_T("PM_Log\\%d\\%d\\%d\\"), time.GetYear(), time.GetMonth(), time.GetDay());
			CString sLogPath = CCommonFunc::GetMakePath(sLogFileName);
			CCommonFunc::CreateDirectoryW(sLogPath);
			sLogFileName.Format(_T("%d.log"), time.GetHour());
			sLogPath += sLogFileName;

			file = _wfopen(sLogPath.GetBuffer(0), _T("ab+"));
			if (file)
			{
				nTime = time.GetHour();
			}
		}
		LeaveCriticalSection(&logFile);
	}

	void log_file_close()
	{
		EnterCriticalSection(&logFile);
		if (file)
		{
			fclose(file);
			file = NULL;
		}
		LeaveCriticalSection(&logFile);
	}

	void log_file_write(char* pValue)
	{
		EnterCriticalSection(&logFile);
		if (file)
		{
			fseek(file, SEEK_END, 0L);
			fwrite(pValue, sizeof(char), strlen(pValue), file);
			fflush(file);		//20230823 GBM - 시스템 출력 버퍼에 바로 적용해서 프로그램 운용 시 비정상종료 시에도 그 이전에 로그는 남기도록 수정
		}
		LeaveCriticalSection(&logFile);
	}

	void WriteLog(INT nMode, LPSTR data, ...)
	{
		SYSTEMTIME	SystemTime;
		CHAR		CurrentDate[32] = { 0, };
		CHAR		CurrentFileName[MAX_PATH] = { 0, };
		FILE*		FilePtr = NULL;
		//CHAR		DebugLog[2048] = { 0, };

		GetLocalTime(&SystemTime);

		if (SystemTime.wHour != nTime)
		{
			log_file_close();
		}
		if (!file)
		{
			log_file_open();
		}

		CHAR szValue[64];
		switch (nMode)
		{
		case 0:
			strcpy_s(szValue, 64, "LOG");
			break;
		case 1:
			strcpy_s(szValue, 64, "DEBUG");
			break;
		case 2:
			strcpy_s(szValue, 64, "WARNING");
			break;
		case 3:
			strcpy_s(szValue, 64, "ERROR");
			break;
		default:
			return;
		}

		sprintf_s(CurrentDate, 32, "%02d/%02d %02d:%02d:%02d %03d",
			/*SystemTime.wYear,*/
			SystemTime.wMonth,
			SystemTime.wDay,
			SystemTime.wHour,
			SystemTime.wMinute,
			SystemTime.wSecond,
			SystemTime.wMilliseconds);

		sprintf_s(szDebugLog[nDebugIndex], 2048, "%s  [ %s ]  %s", CurrentDate, szValue, data);
#ifdef _DEBUG
		OutputDebugStringA(szDebugLog[nDebugIndex]);
#endif

		log_file_write(szDebugLog[nDebugIndex]);
		if (++nDebugIndex >= 10)
		{
			nDebugIndex = 0;
		}
	}

	void Error(const char * fileName, const char * funcName, int line, const char * msg, ...)
	{
		EnterCriticalSection(&logCS);

		static char buffer[BUFFER_SIZE] = {0,};
		va_list args;
		va_start(args, msg);
		vsnprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE-1, msg, args);
		va_end(args);

		char* szText = szLogtext[nLogIndex];
		if (++nLogIndex >= LOG_BUFFER_CNT)
		{
			nLogIndex = 0;
		}
		sprintf(szText, "[file: %s] [Function: %s] [Line: %d] [Error: %s]\n", fileName, funcName, line, buffer);

		WriteLog(2, szText);

		if (pMainWnd)
		{
			pMainWnd->PostMessageW(LOG_MESSAGE, (WPARAM)szText, 3);
		}

		LeaveCriticalSection(&logCS);
	}

	void Error(const char * fileName, const char * funcName, int line, int code, const char * msg, ...)
	{
		EnterCriticalSection(&logCS);

		char* lpMessageBuffer;

		FormatMessageA(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM | 
			FORMAT_MESSAGE_FROM_HMODULE,
			libModule, 
			code,  
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPSTR) &lpMessageBuffer,  
			0,  
			NULL );

		static char buffer[BUFFER_SIZE] = {0,};
		va_list args;
		va_start(args, msg);
		vsnprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE-1, msg, args);
		va_end(args);

		char* szText = szLogtext[nLogIndex];
		if (++nLogIndex >= LOG_BUFFER_CNT)
		{
			nLogIndex = 0;
		}
		sprintf(szText, "[file: %s] [Function: %s] [Line: %d] [Error: %s] [Msg: %s] [Code: %d]\n",
			fileName, funcName, line, buffer, lpMessageBuffer, code);

		WriteLog(2, szText);
		
		if (pMainWnd)
		{
			pMainWnd->PostMessageW(LOG_MESSAGE, (WPARAM)szText, 3);
		}
		/*cout << "File: " << fileName << "\nFunction: " << funcName << "\nLine: " << line \
			<< "\nError: " << buffer << "\nMsg: " << lpMessageBuffer << "Code: " << code << " 0x" << hex << code << endl;*/

		//cout << dec;

		// Free the buffer allocated by the system.
		LocalFree( lpMessageBuffer ); 

		LeaveCriticalSection(&logCS);
	}

	void Trace(const char * msg, ...)
	{
		if( s_Enable )
		{
			EnterCriticalSection(&logCS);

			static char buffer[BUFFER_SIZE] = {0,};
			va_list args;
			va_start(args, msg);
			vsnprintf_s(buffer, BUFFER_SIZE, BUFFER_SIZE-1, msg, args);
			va_end(args);
			
			char* szText = szLogtext[nLogIndex];
			if (++nLogIndex >= LOG_BUFFER_CNT)
			{
				nLogIndex = 0;
			}
			sprintf(szText, "%s\n", buffer);

			WriteLog(0, szText);

			if (pMainWnd)
			{
				pMainWnd->PostMessageW(LOG_MESSAGE, (WPARAM)szText, 0);
			}

			LeaveCriticalSection(&logCS);
		}
	}

	void Setup(CString strLogFolderName)
	{
		InitializeCriticalSection(&logCS);
		InitializeCriticalSection(&logFile);

		libModule = LoadLibraryA("NTDLL.DLL");

		sLogFolderName = strLogFolderName;

		log_file_open();
	}

	void Cleanup()
	{
		EnterCriticalSection(&logCS);

		if(false == FreeLibrary(libModule))
		{
			ERROR_CODE(GetLastError(), "Log::CleanUp() - FreeLibrary() failed.");
		}

		DeleteCriticalSection(&logCS);

		log_file_close();
	}

	void EnableTrace(bool enable)
	{
		s_Enable = enable;
	}
}