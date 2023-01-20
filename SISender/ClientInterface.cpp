#include "stdafx.h"
#include "ClientInterface.h"

CClientInterface::CClientInterface()
{
	m_bConnecting = false;
	m_dwLastTime = 0;
	m_dwLastConnected = 0;
	InitializeCriticalSection(&mConnectionCheck);

	CEncDec::New();
}

CClientInterface::~CClientInterface()
{
	CEncDec::Delete();
}

void CClientInterface::Destroy()
{
	DeleteCriticalSection(&mConnectionCheck);
}

bool CClientInterface::TryConnection(const char* ip, u_short port)
{
	EnterCriticalSection(&mConnectionCheck);

	strcpy_s(m_ip, 32, ip);
	m_port = port;

	if (!m_bConnecting)
	{
		m_bConnecting = true;

		if (Network::Initialize() == false)
		{
			LeaveCriticalSection(&mConnectionCheck);
			m_bConnecting = false;
			return false;
		}
		ClientMan::New();

		ClientMan::Instance()->AddClients(1);
		ClientMan::Instance()->ConnectClients(ip, port);
	}

	LeaveCriticalSection(&mConnectionCheck);
	return true;
}

bool CClientInterface::TryShutdown()
{
	EnterCriticalSection(&mConnectionCheck);

	if (m_bConnecting)
	{
		m_bConnecting = false;

		ClientMan::Instance()->RemoveClients();
		ClientMan::Delete();
		Network::Deinitialize();

		m_dwLastConnected = GetTickCount();
	}
	LeaveCriticalSection(&mConnectionCheck);

	return true;
}

bool CClientInterface::IsConnected()
{
	EnterCriticalSection(&mConnectionCheck);

	if (ClientMan::Instance()->GetClientState() == Client::CONNECTED)
	{
		LeaveCriticalSection(&mConnectionCheck);
		return true;
	}
	LeaveCriticalSection(&mConnectionCheck);
	return false;
}

bool CClientInterface::Send(const char* msg)
{
	EnterCriticalSection(&mConnectionCheck);

	if (ClientMan::Instance()->GetNumClients() > 0)
	{
		ClientMan::Instance()->Send(msg);
	}
	else if(GetTickCount() - m_dwLastConnected >= 3000)
	{
		m_bConnecting = true;

		if (Network::Initialize() == false)
		{
			LeaveCriticalSection(&mConnectionCheck);
			m_bConnecting = false;
			return false;
		}
		ClientMan::New();

		ClientMan::Instance()->AddClients(1);
		ClientMan::Instance()->ConnectClients(m_ip, m_port);

		LeaveCriticalSection(&mConnectionCheck);
		return false;
	}

	LeaveCriticalSection(&mConnectionCheck);

	return true;
}

bool CClientInterface::Send(BYTE* buffer, int nSize)
{
	EnterCriticalSection(&mConnectionCheck);

	if (ClientMan::Instance()->GetNumClients() > 0)
	{
		ClientMan::Instance()->Send(buffer, nSize);
	}
	else if (GetTickCount() - m_dwLastConnected >= 3000)
	{
		m_bConnecting = true;

		if (Network::Initialize() == false)
		{
			LeaveCriticalSection(&mConnectionCheck);
			m_bConnecting = false;
			return false;
		}
		ClientMan::New();

		ClientMan::Instance()->AddClients(1);
		ClientMan::Instance()->ConnectClients(m_ip, m_port);

		LeaveCriticalSection(&mConnectionCheck);
		return false;
	}

	LeaveCriticalSection(&mConnectionCheck);
	return true;
}

void CClientInterface::ProcessCommonRequest(USHORT protocol, int nValue)
{
	ProtocolRequest req;
	req.protocol = protocol;
	req.nValue = nValue;

	if (ProtocolHeader::Keep_Alive != protocol) {
		Log::Trace("ProcessCommonRequest: protocol: %d / value: %d", protocol, nValue);
	}

	Send((BYTE*)&req, sizeof(ProtocolRequest));
}

void CClientInterface::ProcessRequestLogin(WCHAR* szID, WCHAR* szPW)
{
	ProtocolRequestLogin login;
	strcpy_s(login.szID, CCommonFunc::WcharToUtf8(szID));
	strcpy_s(login.szPW, CCommonFunc::WcharToUtf8(szPW));

	Log::Trace("로그인 요청(%s)", login.szID);

	CEncDec::Instance()->PacketEncode((BYTE*)&login, sizeof(ProtocolHeader), sizeof(ProtocolRequestLogin) - sizeof(ProtocolHeader));
	Send((BYTE*)&login, sizeof(ProtocolRequestLogin));
}

void CClientInterface::ProcessRequestAddEvent(int idx, CHAR* szID, CHAR* szEvent, int nCount)
{
	ProtocolRequestAddEvent event;
	event.idx = idx;
	strcpy_s(event.szID, szID);
	strcpy_s(event.szEvent, szEvent);

	Log::Trace("이벤트 정보 저장 요청(요청 갯수: %d)", nCount);// (%s)", CCommonFunc::Utf8ToChar(szEvent));

	Send((BYTE*)&event, sizeof(ProtocolRequestAddEvent));
}

//Kocom

void CClientInterface::KOCOMProcessRequestBind(CHAR* szID, CHAR* szPW)
{
	KOCOMProtocolBind bind;
	bind.header.headerKey = KOCOM_HEADER_KEY_VALUE;
	bind.header.msgType = KOCOM_FIRE_TYPE | KOCOM_MSG_BIND;
	bind.header.msgLength = sizeof(KOCOMProtocolBind) - sizeof(KOCOMProtocolHeader);

	strcpy_s(bind.szId, szID);
	strcpy_s(bind.szPass, szPW);

	Log::Trace("ID : %s, PW : %s Kocom Bind Request", szID, szPW);

	Send((BYTE*)&bind, sizeof(KOCOMProtocolBind));
}

void CClientInterface::KOCOMProcessRequestAlive()
{
	KOCOMProtocolAlive alive;
	alive.header.headerKey = KOCOM_HEADER_KEY_VALUE;
	alive.header.msgType = KOCOM_FIRE_TYPE | KOCOM_MSG_ALIVE;
	alive.header.msgLength = sizeof(KOCOMProtocolAlive) - sizeof(KOCOMProtocolHeader);

	alive.homeVersion = 0;
	alive.nKind = 0;		//프로토콜 문서에서는 별첨을 참조하라고 되어 있는데 별첨이 없음
	alive.nVersion[0] = 0;
	alive.nVersion[1] = 0;
	alive.nVersion[2] = 0;
	alive.nVersion[3] = 0;

	Send((BYTE*)&alive, sizeof(KOCOMProtocolAlive));
}

void CClientInterface::KOCOMProcessRequestFireAlarm(BYTE* pData)
{
	KOCOMProtocolFireAlarm alarm;
	alarm.header.headerKey = KOCOM_HEADER_KEY_VALUE;
	alarm.header.msgType = KOCOM_FIRE_TYPE | KOCOM_MSG_FIRE_ALARM;
	alarm.header.msgLength = sizeof(KOCOMProtocolFireAlarm) - sizeof(KOCOMProtocolHeader);

	//화재 타입 정의
	CString strMsg = _T("");
	int nFireType = 0;
	if (pData[SI_EVENT_BUF_COMMAND] == 'R')
	{
		strMsg = _T("화재 일괄 해제");
		nFireType = KOCOM_FIRE_ALARM_ALL_CLEAR;
	}
	else if((pData[SI_EVENT_BUF_COMMAND] == 'F') && (pData[SI_EVENT_BUF_FIRE_RECEIVER_1] == '0'))
	{
		if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'N')
		{
			strMsg = _T("화재 발생");
			nFireType = KOCOM_FIRE_ALARM_FIRE_OCCURED;
		}
		else if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'F')
		{
			strMsg = _T("화재 해제");
			nFireType = KOCOM_FIRE_ALARM_FIRE_CLEAR;
		}
		else
		{
			return;		// N이나 F가 아니면 리턴
		}
	}
	else
	{
		return;		//화재 타입이나 수신기 복구가 아니면 리턴
	}

	//건물 정보 - 회로 정보 매칭을 통해 위치 정보 확인
	int nFDong = 101;
	int nFHo = 0;
	int nFFloor = 1;

	alarm.nAlarm = nFireType;
	alarm.nFDong = nFDong;
	alarm.nFHo = nFHo;
	alarm.nFFloor = nFFloor;
	strcpy_s(alarm.szFMsg, (CHAR*)pData);

	CHAR cMsg[50];
	memset(cMsg, NULL, 50);
	strcpy_s(cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));

	Log::Trace("%d동 %d호 %d층 - %s", nFDong, nFHo, nFFloor, cMsg);

	Send((BYTE*)&alarm, sizeof(KOCOMProtocolFireAlarm));
}
