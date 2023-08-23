#pragma once

#define		LOG_MESSAGE			(WM_USER+1002)

namespace Log
{
	void Trace(const char * msg, ...);

	void Error(const char* fileName, const char* funcName, int line, const char* msg, ...);
	void Error(const char* fileName, const char* funcName, int line, int code, const char* msg, ...);

	void Setup(CString strLogFolderName);
	void Cleanup();

	void EnableTrace(bool enable);

	void SetMainPointer(CWnd* pWnd);

	void WriteLog(INT nMode, LPSTR data, ...);
}

#define trace0(msg, ...) Log::Trace(msg, __VA_ARGS__);
#define ERROR_MSG(msg, ...) Log::Error(__FILE__, __FUNCTION__, __LINE__, msg, __VA_ARGS__);
#define ERROR_CODE(code, msg, ...) Log::Error(__FILE__, __FUNCTION__, __LINE__, code, msg, __VA_ARGS__);
