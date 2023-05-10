#pragma once

#include "TSingleton.h"
#include "ClientMan.h"
#include <thread>

#include "CommaxSock.h"

using namespace std;

class CClientInterface : public TSingleton<CClientInterface>
{
public:
	CClientInterface();
	~CClientInterface();

public:
	bool TryConnection(const char* ip, u_short port);
	bool TryShutdown();
	bool Send(const char* msg);
	bool Send(BYTE* buffer, int nSize);
	void Destroy();

public:
	void ProcessCommonRequest(USHORT protocol, int nValue);
	void ProcessRequestLogin(WCHAR* szID, WCHAR* szPW);
	void ProcessRequestAddEvent(int idx, CHAR* szID, CHAR* szEvent, int nCount);
	bool IsConnected();

	//Kocom
	void KOCOMProcessRequestBind(CHAR* szID, CHAR* szPW);
	void KOCOMProcessRequestAlive();
	void KOCOMProcessRequestFireAlarm(BYTE* pData);

	//Commax
	void COMMAXProcessRequestFireAlarm(BYTE* pData);

private:
	char m_ip[32];
	u_short m_port;
	bool m_bConnecting;
	DWORD m_dwLastConnected;
	DWORD m_dwLastTime;
	CRITICAL_SECTION mConnectionCheck;

public:
};

