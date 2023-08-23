#include "stdafx.h"
#include "CommaxFunc.h"


CCommaxFunc::CCommaxFunc()
{
}


CCommaxFunc::~CCommaxFunc()
{
}

int CCommaxFunc::MakeSocketNonBlocking(int sock)
{
	unsigned long arg = 1;
	return ioctlsocket(sock, FIONBIO, &arg) == 0;
}

int CCommaxFunc::CheckCommaxResponse(BYTE* pResponse, int nBufLength)
{
	char* pData = (char*)pResponse;

	CString strTemp = _T("");
	strTemp.Format(_T("%s"), g_lpszCommaxTagName[XML_TAG_START_RESPONSE]);
	int nStartTagLength = strTemp.GetLength();
	strTemp.Format(_T("%s"), g_lpszCommaxTagName[XML_TAG_END_RESPONSE]);
	int nEndTagLength = strTemp.GetLength();
	strTemp.Format(_T("%s"), CCommonFunc::CharToTCHAR(pData));
	int nValueLength = nBufLength - (nStartTagLength + nEndTagLength);
	//�������� ��Ŷ�̶�� 1�̾�� �ϹǷ�
	int nResult = -1;
	if (nValueLength != 1)
	{
		return nResult;
	}

	CString strResult = _T("");
	strResult = strTemp.Mid(nStartTagLength, nValueLength);
	nResult = _ttoi(strResult);

	return nResult;
}

void CCommaxFunc::CommaxEventProcess(BYTE* pData)
{
	//
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
	CString strDong = _T("");
	CString strFloor = _T("");
	CString strStair = _T("");

	if (nFireType != COMMAX_FIRE_ALARM_ALL_CLEAR)
	{
		if (nDong != 0)
		{
			strDong.Format(_T("%d"), nDong);
		}

		if (nFloor != 0)
		{
			strFloor.Format(_T("%d"), nFloor);
		}

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
	//

	SOCKET commaxSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (commaxSock == INVALID_SOCKET)
	{
		Log::Trace("Commax Socket Creation Failed!");
		return;
	}

	//Log::Trace("Commax Socket Creation Succeeded!");

	int reuseFlag = 1;
	if (setsockopt(commaxSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseFlag, sizeof(reuseFlag)) != 0)
	{
		Log::Trace("setsockopt(SO_REUSEADDR) error!");
		return;
	}

	//
	MakeSocketNonBlocking(commaxSock);

	//
	sockaddr_in ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(CCommonState::Instance()->m_nPort);

	ServerAddr.sin_addr.s_addr = inet_addr(CCommonState::Instance()->m_szServerIP);
	//

	fd_set set;
	FD_ZERO(&set);
	timeval tvout = { 2, 0 };

	FD_SET(commaxSock, &set);

	int err = 0;

	if (connect(commaxSock, (sockaddr*)&ServerAddr, sizeof(sockaddr_in)) != 0)
	{
		err = WSAGetLastError();
		if ((err != WSAEWOULDBLOCK) && (err != WSAEINTR))
		{
			Log::Trace("connect() Failed!");
			return;
		}

		if (select(commaxSock + 1, NULL, &set, NULL, &tvout) <= 0)
		{
			Log::Trace("select/connect() Failed!");
			Log::Trace("Check your internet network or IP / Port number!");
			return;
		}

		socklen_t len = sizeof(err);
		if (getsockopt(commaxSock, SOL_SOCKET, SO_ERROR, (char*)&err, &len) < 0 || err != 0)
		{
			Log::Trace("getsockopt() error : %d", err);
			closesocket(commaxSock);
			return;
		}
	}
	else
	{
		Log::Trace("Commax Connection Timeout!");
		return;
	}
	CString strConnectionSucceeded = _T("");
	strConnectionSucceeded.Format(_T("Commax Socket Connection Succeeded! - [IP : %s, Port Number : %d]"), CCommonFunc::CharToTCHAR(CCommonState::Instance()->m_szServerIP), CCommonState::Instance()->m_nPort);
	Log::Trace("%s", CCommonFunc::WCharToChar(strConnectionSucceeded.GetBuffer(0)));

	//

	char strUtf8[300];
	memset(strUtf8, NULL, 300);
	strcpy_s(strUtf8, CCommonFunc::WcharToUtf8(strBuf.GetBuffer(0)));
	if (send(commaxSock, strUtf8, strlen(strUtf8), 0) == SOCKET_ERROR)
	{
		DWORD dw;
		dw = WSAGetLastError();
		Log::Trace("Commax Sending Failed!");
		Log::Trace("WSAGetLastError : %d", dw);
		closesocket(commaxSock);
		return;
	}

	CString strFloorPrefix = _T("");
	CString strMsg = _T("");

	switch (nFloorType)
	{
	case FLOOR_TYPE_BASEMENT:
	{
		nFloor *= -1;
		strFloorPrefix = _T("B");
		break;
	}
	case FLOOR_TYPE_RF:
	{
		strFloorPrefix = _T("R");
		break;
	}
	case FLOOR_TYPE_PH:
	{
		strFloorPrefix = _T("PH");
		break;
	}
	case FLOOR_TYPE_PIT:
	{
		strFloorPrefix = _T("PIT");
		break;
	}
	case FLOOR_TYPE_M:
	{
		strFloorPrefix = _T("M");
		break;
	}
	default:
		break;
	}

	if (nFireType != COMMAX_FIRE_ALARM_ALL_CLEAR)
	{
		if (nFloor != 0)
		{
			strMsg.Format(_T("Commax Sending Succeeded! - [# �̺�Ʈ Ÿ�� : %s, # %s ��, # %s%d F, # %d ���]"),
				strFireType,
				strDong,
				strFloorPrefix,
				nFloor,
				nStair);
		}
		else
		{
			strMsg.Format(_T("Commax Sending Succeeded! - [# �̺�Ʈ Ÿ�� : %s, # %s ��, # %s F, # %d ���]"),
				strFireType,
				strDong,
				strFloorPrefix,
				nStair);
		}

	}
	else
	{
		strMsg.Format(_T("Commax Sending Succeeded! - [# �̺�Ʈ Ÿ�� : %s, # ALL (��)]"), strFireType);
	}

	Log::Trace("%s", CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));

	//20230823 - GBM start - ���� ���� ����, COMMAX ������ ������ ���� Ȥ�� ª�� ���� ���� ������ ó������ ���ϹǷ� �ǹ̰� ���� �̷��� ������ �츮 ���α׷��� 2�ʰ� ������� ���¸� ������
// 	LARGE_INTEGER startTime, curTime;
// 	QueryPerformanceCounter(&startTime);
// 
// 	while (true)
// 	{
// 		BYTE buffer[265] = { 0, };
// 		int nLength = 0;
// 		nLength = recv(commaxSock, (char*)buffer, 265, 0);
// 
// 		if (nLength > 0)
// 		{
// 			//Log::Trace("Commax Response Packet : %s", (char*)buffer);
// 			int nResult = 0;
// 			nResult = CheckCommaxResponse(buffer, nLength);
// 			switch (nResult)
// 			{
// 			case -1:
// 			{
// 				Log::Trace("Wrong Packet Received!");
// 				break;
// 			}
// 			case OK:
// 			{
// 				Log::Trace("Commax Response Result : OK!");
// 				break;
// 			}
// 			default:
// 			{
// 				Log::Trace("Commax Response Result : ERROR (%d)!", nResult);
// 				break;
// 			}
// 			}
// 
// 			break;
// 		}
// 
// 		QueryPerformanceCounter(&curTime);
// 		double deltaTime = CCommonFunc::GetPreciseDeltaTime(startTime, curTime);
// 		if (deltaTime > 2000)	//2�� �̳� Response ������ ��ٸ��� �ʰ� ���� ����
// 		{
// 			Log::Trace("Commax Response Timeout!");
// 			break;
// 		}
// 	}
	//20230823 GBM end

	if (closesocket(commaxSock) == SOCKET_ERROR)
	{
		DWORD dw;
		dw = WSAGetLastError();
		Log::Trace("Commax Socket Closing Failed!");
		Log::Trace("WSAGetLastError : %d", dw);
	}

	//Log::Trace("Commax Socket Closed!");

	return;
}
