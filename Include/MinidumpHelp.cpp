#include "StdAfx.h"
#include "MinidumpHelp.h"
#include <time.h>
#include <DbgHelp.h>

#pragma comment(lib,"DbgHelp.Lib")

MinidumpHelp::MinidumpHelp(void)
{
}

MinidumpHelp::~MinidumpHelp(void)
{
}

CString MinidumpHelp::get_dump_filename()
{
	time_t rawtime;
	struct tm timeinfo;

	CString strFileName;
	static WCHAR ModulePath[2048];

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	CString sVersion = CCommonFunc::GetVersionInfo(NULL, L"ProductVersion");

	strFileName.Format(L"%s[%s]%04d%02d%02d_%02dh%02dm%02ds.dmp", 
		CCommonFunc::GetCurrentModuleFileName(), //PROGRAM_SECTION_NAME, 
		sVersion, 
		timeinfo.tm_year + 1900,
		timeinfo.tm_mon + 1,
		timeinfo.tm_mday,
		timeinfo.tm_hour,
		timeinfo.tm_min,
		timeinfo.tm_sec);
	
	// 모듈 경로
	/*::GetModuleFileName(AfxGetApp()->m_hInstance, m_strModulePath.GetBuffer(MAX_PATH), MAX_PATH );
	m_strModulePath.ReleaseBuffer();
	int nPos = m_strModulePath.ReverseFind('\\');
	if( nPos != -1 )
		m_strModulePath = m_strModulePath.Left( nPos );*/
	m_strModulePath = CCommonFunc::GetMakePath(_T("dump\\"));
	//m_strModulePath+="\\dump\\";
	
	CString sPath = m_strModulePath;	

	/*int nStart=0, nEnd=0;
	while((nEnd = sPath.Find('\\', nStart)) >= 0)
	{
		CString folder = sPath.Mid(0, nEnd);
		::CreateDirectory(folder, NULL);
		nStart = nEnd + 1;
	}*/

	m_strModulePath += strFileName;

	return m_strModulePath;
}

LONG WINAPI my_top_level_filter(__in PEXCEPTION_POINTERS pExceptionPointer)
{
	MinidumpHelp mini;
	MINIDUMP_EXCEPTION_INFORMATION MinidumpExceptionInformation;
	CString dump_filename;

	MinidumpExceptionInformation.ThreadId = ::GetCurrentThreadId();
	MinidumpExceptionInformation.ExceptionPointers = pExceptionPointer;
	MinidumpExceptionInformation.ClientPointers = FALSE;


	dump_filename = mini.get_dump_filename();
	if (dump_filename.IsEmpty() == true)
	{
		::TerminateProcess(::GetCurrentProcess(), 0);
	}

	HANDLE hDumpFile = ::CreateFile(dump_filename,
		GENERIC_WRITE, 
		FILE_SHARE_WRITE, 
		NULL, 
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL, NULL);

	MiniDumpWriteDump(GetCurrentProcess(),
		GetCurrentProcessId(),
		hDumpFile,
		MiniDumpNormal,
		&MinidumpExceptionInformation,
		NULL,
		NULL);
	::TerminateProcess(::GetCurrentProcess(), 0);

	return 0;
}

void MinidumpHelp::install_self_mini_dump()
{
	SetUnhandledExceptionFilter(my_top_level_filter);
}
