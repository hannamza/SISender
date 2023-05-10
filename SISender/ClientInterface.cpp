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

	Log::Trace("�α��� ��û(%s)", login.szID);

	CEncDec::Instance()->PacketEncode((BYTE*)&login, sizeof(ProtocolHeader), sizeof(ProtocolRequestLogin) - sizeof(ProtocolHeader));
	Send((BYTE*)&login, sizeof(ProtocolRequestLogin));
}

void CClientInterface::ProcessRequestAddEvent(int idx, CHAR* szID, CHAR* szEvent, int nCount)
{
	ProtocolRequestAddEvent event;
	event.idx = idx;
	strcpy_s(event.szID, szID);
	strcpy_s(event.szEvent, szEvent);

	Log::Trace("�̺�Ʈ ���� ���� ��û(��û ����: %d)", nCount);// (%s)", CCommonFunc::Utf8ToChar(szEvent));

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
	alive.nKind = 0;		//�������� ���������� ��÷�� �����϶�� �Ǿ� �ִµ� ��÷�� ����
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

	//ȭ�� Ÿ�� ����
	CString strFireType = _T("");
	int nFireType = 0;
	if (pData[SI_EVENT_BUF_COMMAND] == 'R')
	{
		strFireType = _T("ȭ�� �ϰ� ����");
		nFireType = KOCOM_FIRE_ALARM_ALL_CLEAR;
	}
	//else if((pData[SI_EVENT_BUF_COMMAND] == 'F') && (pData[SI_EVENT_BUF_FIRE_RECEIVER_1] == '0'))		//��ȣ ������ Ȯ�� ��� �ι�° ������ �ʿ� ����
	else if (pData[SI_EVENT_BUF_COMMAND] == 'F')
	{
		if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'N')
		{
			strFireType = _T("ȭ�� �߻�");
			nFireType = KOCOM_FIRE_ALARM_FIRE_OCCURED;
		}
		else if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'F')
		{
			strFireType = _T("ȭ�� ����");
			nFireType = KOCOM_FIRE_ALARM_FIRE_CLEAR;
		}
		else
		{
			Log::Trace("ȭ�� Ÿ���� �ƴ� �̺�Ʈ�� ���Խ��ϴ�. KOCOM �̺�Ʈ ������ ���� �ʽ��ϴ�.");
			return;		// N�̳� F�� �ƴϸ� ����
		}
	}
	else
	{
		Log::Trace("ȭ�� Ÿ���̳� ���ű� ���� �̿��� �̺�Ʈ�� ���Խ��ϴ�. KOCOM �̺�Ʈ ������ ���� �ʽ��ϴ�.");
		return;		//ȭ�� Ÿ���̳� ���ű� ������ �ƴϸ� ����
	}

	//�ǹ� ���� - ȸ�� ���� ��Ī�� ���� ��ġ ���� Ȯ��
	int nFDong = 101;
	int nFHo = 101;
	int nFFloor = 1;
	int nFloorType = FLOOR_TYPE_ETC;

	//ȸ�ι�ȣ�� �ǹ����� ��Ī
	CString strCircuitNo = _T("");
	strCircuitNo = CCircuitLocInfo::Instance()->GetCircuitNo(pData);

	std::map<CString, CIRCUIT_LOC_INFO>::iterator iter;
	CIRCUIT_LOC_INFO cli;
	//���ű� ������ ��ġ������ ����, 
	if (strCircuitNo.Compare(_T("00000000")) != 0)
	{
		iter = CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.find(strCircuitNo);

		//��ġ�������� ã���� 
		if (iter != CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.end())
		{
			cli = iter->second;
			if (nFireType != KOCOM_FIRE_ALARM_ALL_CLEAR)
			{
				alarm.header.dong = nFDong = atoi(cli.buildingName);
			}

			//�ϴ� ����Ʈ �ǹ�(101��, 102�� ���� �ǹ��� �ƴ� ������ ���� �ǹ��� �̺�Ʈ �Ⱥ������� ���ް� ���ǵ�)
			if (nFDong == 0)
			{
				Log::Trace("����Ʈ �ǹ� ���� ��Ÿ �ǹ��� ȭ�� ������ ���Խ��ϴ�. KOCOM �̺�Ʈ ������ ���� �ʽ��ϴ�.");
				return;
			}

			nFloorType = CCircuitLocInfo::Instance()->CheckFloorType(cli.floor);

			if (nFloorType == FLOOR_TYPE_BASEMENT)
			{	
				nFFloor = atoi(&cli.floor[1]);	// B ���� ���� 
				nFFloor *= -1;
			}
			else if (nFloorType == FLOOR_TYPE_PH)	// ������ ���� ū �ǹ̰� ���� ��ž�� ������ ������ �ϴ� ��ž�� ������ �򵵷� ��
			{
				nFFloor = atoi(&cli.floor[2]);	// PH ���� ����
			}
			else if (nFloorType == FLOOR_TYPE_M)
			{
				nFFloor = atoi(&cli.floor[1]);	// M ���� ���� 
			}
			else // �Ϲ����� �״�� ���� ���, RF�� ������ ���ڰ� ����, ���� Kocom �������ݷ� RF ǥ�� �Ұ���
			{
				nFFloor = atoi(cli.floor);
			}

			//ȣ(��) ����
			alarm.header.ho = nFHo = atoi(cli.room);

		}
	}
	else //���ű� ������ ��� �� ������ ä��
	{
		strcpy(cli.buildingName, "");
		strcpy(cli.circuitName, "");
		strcpy(cli.stair, "");
		strcpy(cli.floor, "");
		strcpy(cli.room, "");
	}
	
	//������
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

	strcpy_s(alarm.szFMsg, CCommonFunc::WcharToUtf8(strMsg.GetBuffer(0)));	//�ѱ����� �� UTF-8 ó��

	CHAR cMsg[80];
	memset(cMsg, NULL, 80);
	strcpy_s(cMsg, CCommonFunc::WCharToChar(strFireType.GetBuffer(0)));

	switch (nFloorType)
	{
	case FLOOR_TYPE_BASEMENT:
	{
		Log::Trace("%d�� B%d�� %dȣ %s [%s]", nFDong, nFFloor * -1, nFHo, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_NORMAL:
	{
		Log::Trace("%d�� %d�� %dȣ %s [%s]", nFDong, nFFloor, nFHo, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_RF:
	{
		Log::Trace("%d�� RF�� %s %s [%s]", nFDong, cli.room, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_PH:
	{
		Log::Trace("%d�� PH%d�� %s %s [%s]", nFDong,  nFFloor, cli.room, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_PIT:
	{
		Log::Trace("%d�� PIT�� %s %s [%s]", nFDong, cli.room, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_M:
	{
		Log::Trace("%d�� M%d�� %sȣ %s [%s]", nFDong, nFFloor, cli.room, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	case FLOOR_TYPE_ETC:
	default:
	{
		Log::Trace("%d�� %s [%s]", nFDong, cMsg, CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		break;
	}
	}

	Send((BYTE*)&alarm, sizeof(KOCOMProtocolFireAlarm));
}

void CClientInterface::COMMAXProcessRequestFireAlarm(BYTE* pData)
{
	//���� ���� �����ϸ� ���� ����
	TryShutdown();

	//��� ����
	TryConnection(CCommonState::Instance()->m_szServerIP, CCommonState::Instance()->m_nPort);

	//���� ����
	//ȭ�� Ÿ�� ����
	CString strFireType = _T("");
	int nFireType = 0;
	if (pData[SI_EVENT_BUF_COMMAND] == 'R')
	{
		strFireType = _T("ȭ�� �ϰ� ����");
		nFireType = COMMAX_FIRE_ALARM_ALL_CLEAR;
	}
	//else if((pData[SI_EVENT_BUF_COMMAND] == 'F') && (pData[SI_EVENT_BUF_FIRE_RECEIVER_1] == '0'))		//��ȣ ������ Ȯ�� ��� �ι�° ������ �ʿ� ����
	else if (pData[SI_EVENT_BUF_COMMAND] == 'F')
	{
		if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'N')
		{
			strFireType = _T("ȭ�� �߻�");
			nFireType = COMMAX_FIRE_ALARM_FIRE_OCCURED;
		}
		else if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'F')
		{
			strFireType = _T("ȭ�� ����");
			nFireType = COMMAX_FIRE_ALARM_FIRE_CLEAR;
		}
		else
		{
			Log::Trace("ȭ�� Ÿ���� �ƴ� �̺�Ʈ�� ���Խ��ϴ�. COMMAX �̺�Ʈ ������ ���� �ʽ��ϴ�.");
			return;		// N�̳� F�� �ƴϸ� ����
		}
	}
	else
	{
		Log::Trace("ȭ�� Ÿ���̳� ���ű� ���� �̿��� �̺�Ʈ�� ���Խ��ϴ�. COMMAX �̺�Ʈ ������ ���� �ʽ��ϴ�.");
		return;		//ȭ�� Ÿ���̳� ���ű� ������ �ƴϸ� ����
	}

	//�ǹ� ���� - ȸ�� ���� ��Ī�� ���� ��ġ ���� Ȯ��
	int nDong = 0;
	int nFloor = 0;
	int nStair = 0;
	int nFloorType = FLOOR_TYPE_ETC;

	//ȸ�ι�ȣ�� �ǹ����� ��Ī
	CString strCircuitNo = _T("");
	strCircuitNo = CCircuitLocInfo::Instance()->GetCircuitNo(pData);

	std::map<CString, CIRCUIT_LOC_INFO>::iterator iter;
	CIRCUIT_LOC_INFO cli;
	//���ű� ������ ��ġ������ ����, 
	if (strCircuitNo.Compare(_T("00000000")) != 0)
	{
		iter = CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.find(strCircuitNo);

		//��ġ�������� ã���� 
		if (iter != CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.end())
		{
			cli = iter->second;
			if (nFireType != COMMAX_FIRE_ALARM_ALL_CLEAR)
			{
				nDong = atoi(cli.buildingName);
			}

			//�ϴ� ����Ʈ �ǹ�(101��, 102�� ���� �ǹ��� �ƴ� ������ ���� �ǹ��� ���۾���)
			if (nDong == 0)
			{
				Log::Trace("����Ʈ �ǹ� ���� ��Ÿ �ǹ��� ȭ�� ������ ���Խ��ϴ�. COMMAX �̺�Ʈ ������ ���� �ʽ��ϴ�.");
				return;
			}

			nFloorType = CCircuitLocInfo::Instance()->CheckFloorType(cli.floor);

			if (nFloorType == FLOOR_TYPE_BASEMENT)
			{
				nFloor = atoi(&cli.floor[1]);	// B ���� ���� 
				nFloor *= -1;
			}
			else if (nFloorType == FLOOR_TYPE_PH)	// COMMAX�� ���� ū �ǹ̰� ���� ��ž�� ������ ������ �ϴ� ��ž�� ������ �򵵷� ��
			{
				nFloor = atoi(&cli.floor[2]);	// PH ���� ����
			}
			else if (nFloorType == FLOOR_TYPE_M)
			{
				nFloor = atoi(&cli.floor[1]);	// M ���� ���� 
			}
			else // �Ϲ����� �״�� ���� ���, RF�� ������ ���ڰ� ����, ���� COMMAX �������ݷ� RF ǥ�� �Ұ���
			{
				nFloor = atoi(cli.floor);
			}

			nStair = atoi(cli.stair);

			//ȣ(��) ������ COMMAX �������ݿ� ����
		}

	}
	else //���ű� ����
	{

	}

	CString strBuf = _T("");

	if (nFireType != COMMAX_FIRE_ALARM_ALL_CLEAR)
	{
		CString strDong = _T("");
		if (nDong != 0)
		{
			strDong.Format(_T("%d"), nDong);
		}

		CString strFloor = _T("");
		if (nFloor != 0)
		{
			strFloor.Format(_T("%d"), nFloor);
		}

		CString strStair = _T("");
		if (nStair != 0)
		{
			strStair.Format(_T("%d"), nStair);
		}

		strBuf.Format(_T("%s\r\n\t%s\r\n\t\t%s%d%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t%s\r\n%s")
			, g_lpszCommaxTagName[XML_TAG_START_COMMAX]
			, g_lpszCommaxTagName[XML_TAG_START_FIRE]
			, g_lpszCommaxTagName[XML_TAG_START_EMERGENCY]
			, nFireType
			, g_lpszCommaxTagName[XML_TAG_END_EMERGENCY]
			, g_lpszCommaxTagName[XML_TAG_START_DONG]
			, strDong
			, g_lpszCommaxTagName[XML_TAG_END_DONG]
			, g_lpszCommaxTagName[XML_TAG_START_FLOOR]
			, strFloor
			, g_lpszCommaxTagName[XML_TAG_END_FLOOR]
			, g_lpszCommaxTagName[XML_TAG_START_STAIR]
			, strStair
			, g_lpszCommaxTagName[XML_TAG_END_STAIR]
			, g_lpszCommaxTagName[XML_TAG_START_LINE]
			, _T("")
			, g_lpszCommaxTagName[XML_TAG_END_LINE]
			, g_lpszCommaxTagName[XML_TAG_END_FIRE]
			, g_lpszCommaxTagName[XML_TAG_END_COMMAX]
		);
	}
	else // COMMAX���� ���ű� ������ ȭ������(2), ���� ALL�� �����޶�� ��û��
	{
		strBuf.Format(_T("%s\r\n\t%s\r\n\t\t%s%d%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t%s\r\n%s")
			, g_lpszCommaxTagName[XML_TAG_START_COMMAX]
			, g_lpszCommaxTagName[XML_TAG_START_FIRE]
			, g_lpszCommaxTagName[XML_TAG_START_EMERGENCY]
			, COMMAX_FIRE_ALARM_FIRE_CLEAR
			, g_lpszCommaxTagName[XML_TAG_END_EMERGENCY]
			, g_lpszCommaxTagName[XML_TAG_START_DONG]
			, _T("ALL")
			, g_lpszCommaxTagName[XML_TAG_END_DONG]
			, g_lpszCommaxTagName[XML_TAG_START_FLOOR]
			, _T("")
			, g_lpszCommaxTagName[XML_TAG_END_FLOOR]
			, g_lpszCommaxTagName[XML_TAG_START_STAIR]
			, _T("")
			, g_lpszCommaxTagName[XML_TAG_END_STAIR]
			, g_lpszCommaxTagName[XML_TAG_START_LINE]
			, _T("")
			, g_lpszCommaxTagName[XML_TAG_END_LINE]
			, g_lpszCommaxTagName[XML_TAG_END_FIRE]
			, g_lpszCommaxTagName[XML_TAG_END_COMMAX]
		);
	}

	//Send
	int nClientListSize = 0;
	nClientListSize = ClientMan::Instance()->GetNumClients();
	HANDLE hHandle[100];

	for (int i = 0; i < nClientListSize; i++)
	{
		hHandle[i] = ClientMan::Instance()->m_listClient[i]->m_hCommaxConnect;
	}

	DWORD dw = WaitForMultipleObjects(nClientListSize, hHandle, TRUE, 3000);
	if (dw != WAIT_OBJECT_0)
	{
		int a = 0;
	}

	if (m_bConnecting)
	{
		char cBuf[300];
		memset(cBuf, NULL, 300);
		strcpy(cBuf, CCommonFunc::WCharToChar(strBuf.GetBuffer(0)));
		Send((BYTE*)cBuf, strlen(cBuf));
	}
	else
	{
		Log::Trace("����� ������� �ʾ� ȭ�� �̺�Ʈ�� �������� �ʽ��ϴ�.");
	}

	//��� ����
	TryShutdown();
}