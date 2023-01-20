#pragma once

#include "TSingleton.h"

class CCommonState : public TSingleton<CCommonState>
{
public:
	CCommonState();
	~CCommonState();

public:
	bool m_bLoginResult;
	int m_nProcessIndex;
	int m_nSIType;
	char m_szServerIP[32];
	int m_nPort;
	int m_nIdx;
	char m_szID[40];
	char m_szPass[40];
	DWORD m_dwToken;
	DWORD m_dwLastRecv;
};

