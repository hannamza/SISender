#pragma once

#include "TSingleton.h"

class CCommonState : public TSingleton<CCommonState>
{
public:
	CCommonState();
	~CCommonState();

public:
	bool m_bLoginResult;
	int m_nIdx;
	char m_szID[32];
	DWORD m_dwToken;
	DWORD m_dwLastRecv;
};

