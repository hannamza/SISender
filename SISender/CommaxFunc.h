#pragma once
class CCommaxFunc:public TSingleton<CCommaxFunc>
{
public:
	CCommaxFunc();
	~CCommaxFunc();

public:
	int MakeSocketNonBlocking(int sock);
	int CheckCommaxResponse(BYTE* pResponse, int nBufLength);
	void CommaxEventProcess(BYTE* pData);
};

