// CommaxSock.cpp : ���� �����Դϴ�.
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


// CCommaxSock ��� �Լ�


void CCommaxSock::OnConnect(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
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
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	CAsyncSocket::OnReceive(nErrorCode);
}
