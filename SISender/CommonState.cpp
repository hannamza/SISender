#include "stdafx.h"
#include "CommonState.h"


CCommonState::CCommonState()
{
	m_bLoginResult = false;
	m_nProcessIndex = 0;
	m_nSIType = 0;
	m_nIdx = 0;
	m_szID[0] = '\0';
	m_dwToken = 0;
	m_dwLastRecv = GetTickCount();
}


CCommonState::~CCommonState()
{
}
