// CommaxSock.cpp : 구현 파일입니다.
//
#include "stdafx.h"
#include "SISender.h"
#include "CommaxSock.h"


// CCommaxSock

CCommaxSock::CCommaxSock()
{
}

CCommaxSock::~CCommaxSock()
{
}


// CCommaxSock 멤버 함수


void CCommaxSock::OnConnect(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (nErrorCode != 0)
	{
		CString str;
		str.Format(_T("Error Code : %d"), nErrorCode);
		AfxMessageBox(str);
	}
	
	CAsyncSocket::OnConnect(nErrorCode);
}


void CCommaxSock::OnReceive(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CAsyncSocket::OnReceive(nErrorCode);
}
