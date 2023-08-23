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
	//정상적인 패킷이라면 1이어야 하므로
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
	//버퍼 가공
	//화재 타입 정의
	CString strFireType = _T("");
	int nFireType = 0;
	if (pData[SI_EVENT_BUF_COMMAND] == 'R')
	{
		strFireType = _T("화재 일괄 해제");
		nFireType = COMMAX_FIRE_ALARM_ALL_CLEAR;
	}
	//else if((pData[SI_EVENT_BUF_COMMAND] == 'F') && (pData[SI_EVENT_BUF_FIRE_RECEIVER_1] == '0'))		//김호 마스터 확인 결과 두번째 조건을 필요 없음
	else if (pData[SI_EVENT_BUF_COMMAND] == 'F')
	{
		if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'N')
		{
			strFireType = _T("화재 발생");
			nFireType = COMMAX_FIRE_ALARM_FIRE_OCCURED;
		}
		else if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'F')
		{
			strFireType = _T("화재 해제");
			nFireType = COMMAX_FIRE_ALARM_FIRE_CLEAR;
		}
		else
		{
			Log::Trace("화재 타입이 아닌 이벤트가 들어왔습니다. COMMAX 이벤트 전송을 하지 않습니다.");
			return;		// N이나 F가 아니면 리턴
		}
	}
	else
	{
		Log::Trace("화재 타입이나 수신기 복구 이외의 이벤트가 들어왔습니다. COMMAX 이벤트 전송을 하지 않습니다.");
		return;		//화재 타입이나 수신기 복구가 아니면 리턴
	}

	//건물 정보 - 회로 정보 매칭을 통해 위치 정보 확인
	int nDong = 0;
	int nFloor = 0;
	int nStair = 0;
	int nFloorType = FLOOR_TYPE_ETC;

	//회로번호와 건물정보 매칭
	CString strCircuitNo = _T("");
	strCircuitNo = CCircuitLocInfo::Instance()->GetCircuitNo(pData);

	std::map<CString, CIRCUIT_LOC_INFO>::iterator iter;
	CIRCUIT_LOC_INFO cli;
	//수신기 복구는 위치정보에 없음, 
	if (strCircuitNo.Compare(_T("00000000")) != 0)
	{
		iter = CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.find(strCircuitNo);

		//위치정보에서 찾으면 
		if (iter != CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.end())
		{
			cli = iter->second;
			if (nFireType != COMMAX_FIRE_ALARM_ALL_CLEAR)
			{
				nDong = atoi(cli.buildingName);
			}

			//일단 아파트 건물(101동, 102동 등의 건물이 아닌 주차장 등의 건물은 전송안함)
			if (nDong == 0)
			{
				Log::Trace("아파트 건물 외의 기타 건물의 화재 정보가 들어왔습니다. COMMAX 이벤트 전송을 하지 않습니다.");
				return;
			}

			nFloorType = CCircuitLocInfo::Instance()->CheckFloorType(cli.floor);

			if (nFloorType == FLOOR_TYPE_BASEMENT)
			{
				nFloor = atoi(&cli.floor[1]);	// B 이후 숫자 
				nFloor *= -1;
			}
			else if (nFloorType == FLOOR_TYPE_PH)	// COMMAX는 층이 큰 의미가 없고 옥탑층 개념이 없지만 일단 옥탑층 정보를 얻도록 함
			{
				nFloor = atoi(&cli.floor[2]);	// PH 이후 숫자
			}
			else if (nFloorType == FLOOR_TYPE_M)
			{
				nFloor = atoi(&cli.floor[1]);	// M 이후 숫자 
			}
			else // 일반층은 그대로 층수 얻고, RF는 어차피 숫자가 없음, 현재 COMMAX 프로토콜로 RF 표현 불가능
			{
				nFloor = atoi(cli.floor);
			}

			nStair = atoi(cli.stair);

			//호(실) 정보는 COMMAX 프로토콜에 없음
		}

	}
	else //수신기 복구
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
	else // COMMAX에서 수신기 복구를 화재해제(2), 동을 ALL로 보내달라고 요청함
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
			strMsg.Format(_T("Commax Sending Succeeded! - [# 이벤트 타입 : %s, # %s 동, # %s%d F, # %d 계단]"),
				strFireType,
				strDong,
				strFloorPrefix,
				nFloor,
				nStair);
		}
		else
		{
			strMsg.Format(_T("Commax Sending Succeeded! - [# 이벤트 타입 : %s, # %s 동, # %s F, # %d 계단]"),
				strFireType,
				strDong,
				strFloorPrefix,
				nStair);
		}

	}
	else
	{
		strMsg.Format(_T("Commax Sending Succeeded! - [# 이벤트 타입 : %s, # ALL (동)]"), strFireType);
	}

	Log::Trace("%s", CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));

	//20230823 - GBM start - 응답 받지 않음, COMMAX 서버가 어차피 동시 혹은 짧은 순간 많은 연결을 처리하지 못하므로 의미가 없고 이러한 행정은 우리 프로그램이 2초간 응답없음 상태를 유발함
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
// 		if (deltaTime > 2000)	//2초 이내 Response 없으면 기다리지 않고 소켓 종료
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
