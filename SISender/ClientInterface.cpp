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

	Log::Trace("?????? ????(%s)", login.szID);

	CEncDec::Instance()->PacketEncode((BYTE*)&login, sizeof(ProtocolHeader), sizeof(ProtocolRequestLogin) - sizeof(ProtocolHeader));
	Send((BYTE*)&login, sizeof(ProtocolRequestLogin));
}

void CClientInterface::ProcessRequestAddEvent(int idx, CHAR* szID, CHAR* szEvent, int nCount)
{
	ProtocolRequestAddEvent event;
	event.idx = idx;
	strcpy_s(event.szID, szID);
	strcpy_s(event.szEvent, szEvent);

	Log::Trace("?????? ???? ???? ????(???? ????: %d)", nCount);// (%s)", CCommonFunc::Utf8ToChar(szEvent));

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
	alive.nKind = 0;		//???????? ?????????? ?????? ?????????? ???? ?????? ?????? ????
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
	alarm.header.town = 0;
	alarm.header.dong = 101;
	alarm.header.ho = 0;
	alarm.header.reserved = 0;

	//???? ???? ????
	CString strFireType = _T("");
	int nFireType = 0;
	if (pData[SI_EVENT_BUF_COMMAND] == 'R')
	{
		strFireType = _T("???? ???? ????");
		nFireType = KOCOM_FIRE_ALARM_ALL_CLEAR;
	}
	//else if((pData[SI_EVENT_BUF_COMMAND] == 'F') && (pData[SI_EVENT_BUF_FIRE_RECEIVER_1] == '0'))		//???? ?????? ???? ???? ?????? ?????? ???? ????
	else if (pData[SI_EVENT_BUF_COMMAND] == 'F')
	{
		if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'N')
		{
			strFireType = _T("???? ????");
			nFireType = KOCOM_FIRE_ALARM_FIRE_OCCURED;
		}
		else if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'F')
		{
			strFireType = _T("???? ????");
			nFireType = KOCOM_FIRE_ALARM_FIRE_CLEAR;
		}
		else
		{
			Log::Trace("???? ?????? ???? ???????? ????????????. KOCOM ?????? ?????? ???? ????????.");
			return;		// N???? F?? ?????? ????
		}
	}
	else
	{
		Log::Trace("???? ???????? ?????? ???? ?????? ???????? ????????????. KOCOM ?????? ?????? ???? ????????.");
		return;		//???? ???????? ?????? ?????? ?????? ????
	}

	//???? ???? - ???? ???? ?????? ???? ???? ???? ????
	int nFDong = 101;
	int nFHo = 101;
	int nFFloor = 1;
	int nFloorType = FLOOR_TYPE_ETC;

	//?????????? ???????? ????
	CString strCircuitNo = _T("");
	strCircuitNo = CCircuitLocInfo::Instance()->GetCircuitNo(pData);

	std::map<CString, CIRCUIT_LOC_INFO>::iterator iter;
	CIRCUIT_LOC_INFO cli;
	//?????? ?????? ?????????? ????, 
	if (strCircuitNo.Compare(_T("00000000")) != 0)
	{
		iter = CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.find(strCircuitNo);

		//???????????? ?????? 
		if (iter != CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.end())
		{
			cli = iter->second;
			if (nFireType != KOCOM_FIRE_ALARM_ALL_CLEAR)
			{
				alarm.header.dong = nFDong = atoi(cli.buildingName);
			}

			//???? ?????? ????(101??, 102?? ???? ?????? ???? ?????? ???? ?????? ?????? ?????????? ?????? ??????)
			if (nFDong == 0)
			{
				Log::Trace("?????? ???? ???? ???? ?????? ???? ?????? ????????????. KOCOM ?????? ?????? ???? ????????.");
				return;
			}

			nFloorType = CCircuitLocInfo::Instance()->CheckFloorType(cli.floor);

			if (nFloorType == FLOOR_TYPE_BASEMENT)
			{	
				nFFloor = atoi(&cli.floor[1]);	// B ???? ???? 
				nFFloor *= -1;
			}
			else if (nFloorType == FLOOR_TYPE_PH)	// ?????? ???? ?? ?????? ???? ?????? ?????? ?????? ???? ?????? ?????? ?????? ??
			{
				nFFloor = atoi(&cli.floor[2]);	// PH ???? ????
			}
			else if (nFloorType == FLOOR_TYPE_M)
			{
				nFFloor = atoi(&cli.floor[1]);	// M ???? ???? 
			}
			else // ???????? ?????? ???? ????, RF?? ?????? ?????? ????, ???? Kocom ?????????? RF ???? ??????
			{
				nFFloor = atoi(cli.floor);
			}

			//??(??) ????
			alarm.header.ho = nFHo = atoi(cli.room);

		}
	}
	else //?????? ?????? ???? ?? ?????? ????
	{
		strcpy(cli.buildingName, "");
		strcpy(cli.circuitName, "");
		strcpy(cli.stair, "");
		strcpy(cli.floor, "");
		strcpy(cli.room, "");
	}
	
	//??????
	alarm.nAlarm = nFireType;
	alarm.nFDong = nFDong;
	alarm.nFHo = nFHo;
	alarm.nFFloor = nFFloor;

	CString strMsg = _T("");
	CString strTemp = _T("");

	strMsg = _T("#dong:");
	strTemp.Format(_T("%s"), CCommonFunc::CharToWCHAR(cli.buildingName));
	strMsg += strTemp;
	strMsg += _T(",#ho:");
	strTemp.Format(_T("%s"), CCommonFunc::CharToWCHAR(cli.room));
	strMsg += strTemp;
	strMsg += _T(",#floor:");
	strTemp.Format(_T("%s"), CCommonFunc::CharToWCHAR(cli.floor));
	strMsg += strTemp;

	strcpy_s(alarm.szFMsg, CCommonFunc::WcharToUtf8(strMsg.GetBuffer(0)));	//???????? ?? UTF-8 ????

	CHAR cMsg[80];
	memset(cMsg, NULL, 80);
	strcpy_s(cMsg, CCommonFunc::WCharToChar(strFireType.GetBuffer(0)));

	switch (nFloorType)
	{
	case FLOOR_TYPE_BASEMENT:
	{
		Log::Trace("%d?? B%d?? %d?? %s [%s]", nFDong, nFFloor * -1, nFHo, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_NORMAL:
	{
		Log::Trace("%d?? %d?? %d?? %s [%s]", nFDong, nFFloor, nFHo, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_RF:
	{
		Log::Trace("%d?? RF?? %s %s [%s]", nFDong, cli.room, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_PH:
	{
		Log::Trace("%d?? PH%d?? %s %s [%s]", nFDong,  nFFloor, cli.room, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_PIT:
	{
		Log::Trace("%d?? PIT?? %s %s [%s]", nFDong, cli.room, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_M:
	{
		Log::Trace("%d?? M%d?? %s?? %s [%s]", nFDong, nFFloor, cli.room, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_ETC:
	default:
	{
		Log::Trace("%d?? %s [%s]", nFDong, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	}

	Send((BYTE*)&alarm, sizeof(KOCOMProtocolFireAlarm));
}
