#pragma once
#include "TSingleton.h"

class CCommonData : public TSingleton<CCommonData>
{
public:
	CCommonData();
	~CCommonData();

public:
	int m_nLimitClient;
	int m_nServerPort;
	char m_szID[40];
	char m_szPW[40];
};

