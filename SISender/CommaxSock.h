#pragma once

#include "stdafx.h"
#include "SISender.h"

// CCommaxSock 명령 대상입니다.
class CCommaxSock : public CAsyncSocket
{
public:
	CCommaxSock();
	virtual ~CCommaxSock();

	virtual void OnConnect(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
};


