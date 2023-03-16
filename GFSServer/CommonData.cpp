#include "stdafx.h"
#include "CommonData.h"


CCommonData::CCommonData()
{
	m_nLimitClient = 1000;
	m_nServerPort = 10234;
	memset(m_szID, NULL, 40);
	strcpy(m_szID, "gfs");
	memset(m_szPW, NULL, 40);
	strcpy(m_szPW, "gfs");
}


CCommonData::~CCommonData()
{
}
