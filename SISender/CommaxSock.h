#pragma once

#include "stdafx.h"
#include "SISender.h"

// CCommaxSock ��� ����Դϴ�.
class CCommaxSock : public CAsyncSocket
{
public:
	CCommaxSock();
	virtual ~CCommaxSock();

	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
};


