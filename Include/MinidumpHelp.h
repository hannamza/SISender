// http://wiredlayer.tistory.com/20에 나온 소스를 간단하게 클래스로 변경해놓기만 한 파일입니다^^
#pragma once

#include <Windows.h>
#include <string>

LONG WINAPI my_top_level_filter(__in PEXCEPTION_POINTERS pExceptionPointer);
class MinidumpHelp
{
private:
	CString m_strModulePath;	
public:
	CString get_dump_filename();
	MinidumpHelp(void);
	~MinidumpHelp(void);
	
	void install_self_mini_dump();

};
