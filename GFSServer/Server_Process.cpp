#include "stdafx.h"
#include "Server.h"
#include "Client.h"
#include "Packet.h"
#include "IOEvent.h"

#include "Log.h"
#include "Network.h"
#include <iostream>
#include <cassert>
#include <algorithm>
//#include "KISA_SEED_CBC.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CALLBACK Server::WorkerProcessRecvPacket(PTP_CALLBACK_INSTANCE /* Instance */, PVOID Context)
{
	Packet* packet = static_cast<Packet*>(Context);
	assert(packet);

	BYTE* buffer = packet->GetData(), *pData = NULL;

	UINT command = 0;
	UINT size = 0;
	memcpy_s(&command, sizeof(UINT), buffer, sizeof(UINT));
	memcpy_s(&size, sizeof(UINT), &buffer[sizeof(UINT)], sizeof(UINT));

	if (command < GFSProtocolHeader::GFSProtocolCommand::ERROR_RESPONSE || command > GFSProtocolHeader::GFSProtocolCommand::UNSOLICITED_EVENT)
	{
		Server::Instance()->ProcessGFSProtocolResponseError(GFSProtocolHeader::GFSProtocolResult::UNKNOWN_COMMAND, packet);
		return;
	}

	if (size > 0)
	{
		pData = buffer;
	}
	else
	{
		Server::Instance()->ProcessGFSProtocolResponseError(GFSProtocolHeader::GFSProtocolResult::WRONG_PACKET_SIZE, packet);
		return;
	}


	switch (command)
	{
	case GFSProtocolHeader::GFSProtocolCommand::LOGIN_REQUEST:
	{
		if (size != sizeof(GFSProtocolLoginRequest))
		{
			Server::Instance()->ProcessGFSProtocolResponseError(GFSProtocolHeader::GFSProtocolResult::WRONG_PACKET_SIZE, packet);
			return;
		}

		ProcessGFSProtocolRequestLogin(pData, packet);
		break;
	}
	case GFSProtocolHeader::GFSProtocolCommand::KEEP_ALIVE_REQUEST:
	{
		if (size != sizeof(GFSProtocolKeepAliveRequest))
		{
			Server::Instance()->ProcessGFSProtocolResponseError(GFSProtocolHeader::GFSProtocolResult::WRONG_PACKET_SIZE, packet);
			return;
		}

		ProcessGFSProtocolRequestKeepAlive(pData, packet);
		break;
	}
		
#if 0
	case ProtocolHeader::Keep_Alive:
		Server::Instance()->Echo(packet);
		//trace0("[%d] KEEP ALIVE Packet.", GetCurrentThreadId());
		break;
	case ProtocolHeader::RequestLogin:				// login(manager)
		ProcessProtocolRequestLogin(pData, packet);
		break;
	case ProtocolHeader::RequestManagerInfo:
		ProcessProtocolRequestManagerInfo(pData, packet);
		break;
	case ProtocolHeader::RequestAddEvent:
		ProcessProtocolRequestAddEvent(pData, packet);
		break;
	case ProtocolHeader::RequestSetToken:			// 토큰, 전화번호 전송
		ProcessProtocolRequestSetToken(pData, packet);
		break;
	case ProtocolHeader::RequestGetEventList:		// 일련번호 이후의 이벤트 리스트 요청
		ProcessProtocolRequestGetEventList(pData, packet);
		break;
	case ProtocolHeader::RequestAdminLogin:			// 관리자 로그인
		ProcessProtocolRequestAdminLogin(pData, packet);
		break;
	case ProtocolHeader::RequestAddNewSystem:		// 건물, 단지 등 시스템 추가
		ProcessProtocolRequestAddNewSystem(pData, packet);
		break;
	case ProtocolHeader::RequestModSystem:			// 시스템 수정
		ProcessProtocolRequestModSystem(pData, packet);
		break;
	case ProtocolHeader::RequestDelSystem:			// 시스템 삭제
		ProcessProtocolRequestDelSystem(pData, packet);
		break;
	case ProtocolHeader::RequestGetSystemList:		// 시스템 목록 요청
		ProcessProtocolRequestGetSystemList(pData, packet);
		break;
	case ProtocolHeader::RequestGetManagerList:		// 매니저 목록 요청
		ProcessProtocolRequestGetManagerList(pData, packet);
		break;
	case ProtocolHeader::RequestAddManager:			// 매니저 추가
		ProcessProtocolRequestAddManager(pData, packet);
		break;
	case ProtocolHeader::RequestModManager:			// 매니저 수정
		ProcessProtocolRequestModManager(pData, packet);
		break;
	case ProtocolHeader::RequestDelManager:			// 매니저 삭제
		ProcessProtocolRequestDelManager(pData, packet);
		break;
	case ProtocolHeader::RequestGetUserList:		// 해당 시스템에 등록된 유저 목록 요청
		ProcessProtocolRequestGetUserList(pData, packet);
		break;
	case ProtocolHeader::RequestGetUserTokenList:	// 해당 시스템에 등록된 유저 토큰 목록 요청
		ProcessProtocolRequestGetUserTokenList(pData, packet);
		break;
	case ProtocolHeader::RequestAddUser:			// 유저 추가
		ProcessProtocolRequestAddUser(pData, packet);
		break;
	case ProtocolHeader::RequestModUser:			// 유저 수정
		ProcessProtocolRequestModUser(pData, packet);
		break;
	case ProtocolHeader::RequestDelUser:			// 유저 삭제
		ProcessProtocolRequestDelUser(pData, packet);
		break;
	case ProtocolHeader::RequestPhoneToken:
		ProcessProtocolRequestSetToken1(pData, packet);
		break;
	case ProtocolHeader::RequestGetEventListEnc:
		ProcessProtocolRequestGetEventListEnc(pData, packet);
		break;
#endif
	default:
		Server::Instance()->Echo(packet);
		break;
	}
}

#if 0
void Server::ProcessProtocolRequestManagerInfo(BYTE* pData, Packet* packet)
{
	CEncDec::Instance()->PacketDecode(pData, sizeof(ProtocolHeader), sizeof(ProtocolRequestLogin) - sizeof(ProtocolHeader));
	ProtocolRequestLogin* pReq = (ProtocolRequestLogin*)pData;
	ProtocolResponseManagerInfo res;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;

	sprintf_s(szQuery, 2048, "select seq, worksite_seq, user_limit from manager_account where id='%s' and pw=PASSWORD('%s') and isuse='1'", pReq->szID, pReq->szPW);
	pRes = pManager->MysqlSelectQuery(szQuery);

	// 존재 하면 성공
	if (pRes && (UINT)mysql_num_rows(pRes) == 1)
	{
		TRACE("success\n");
		while (ContentRow = mysql_fetch_row(pRes))
		{
			res.nResult = 1;
			res.nManagerSeq = CCommonFunc::GetAtoUI((char*)ContentRow[0]);
			res.nWorkSiteSeq = CCommonFunc::GetAtoUI((char*)ContentRow[1]);
			res.nLimitUser = CCommonFunc::GetAtoUI((char*)ContentRow[2]);
		}
	}
	else
	{
		res.nResult = 0;
	}
	pManager->ReleaseSelectQuery(pRes);

	CDBPool::Instance()->ReturnDBManager(pManager);

	trace0("ProcessProtocolRequestManagerInfo- user: %s, result: %d", pReq->szID, res.nResult);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponseManagerInfo));
}

void Server::ProcessProtocolRequestLogin(BYTE* pData, Packet* packet)	// login(manager)
{
	CEncDec::Instance()->PacketDecode(pData, sizeof(ProtocolHeader), sizeof(ProtocolRequestLogin) - sizeof(ProtocolHeader));
	ProtocolRequestLogin* pReq = (ProtocolRequestLogin*)pData;
	ProtocolResponse res;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;

	sprintf_s(szQuery, 2048, "select seq from manager_account where id='%s' and pw=PASSWORD('%s') and isuse='1'", pReq->szID, pReq->szPW);
	pRes = pManager->MysqlSelectQuery(szQuery);

	// 존재 하면 성공
	if (pRes && (UINT)mysql_num_rows(pRes) == 1)
	{
		TRACE("success\n");
		while (ContentRow = mysql_fetch_row(pRes))
		{
			res.nResult = CCommonFunc::GetAtoUI((char*)ContentRow[0]);
		}
	}
	else
	{
		res.nResult = 0;
	}
	pManager->ReleaseSelectQuery(pRes);

	CDBPool::Instance()->ReturnDBManager(pManager);

	res.protocol = ProtocolHeader::ResponseLogin;

	trace0("ProcessProtocolRequestLogin- user: %s, result: %d", pReq->szID, res.nResult);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
}

void Server::ProcessProtocolRequestSetToken(BYTE* pData, Packet* packet)	// 토큰, 전화번호 전송
{
	ProtocolRequestSetToken* pReq = (ProtocolRequestSetToken*)pData;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	bool bUpdate = false;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT token FROM token_info WHERE phone='%s'", pReq->szPhoneNo);
	pRes = pManager->MysqlSelectQuery(szQuery);
	if (pRes)
	{
		if (pRes && mysql_num_rows(pRes) > 0)
		{
			/*if (ContentRow = mysql_fetch_row(pRes))
			{
				if (strcmp(pReq->szToken, (char*)ContentRow[0]) == 0)
				{
					pManager->ReleaseSelectQuery(pRes);
					CDBPool::Instance()->ReturnDBManager(pManager);
					return;
				}
			}*/
			bUpdate = true;
		}
		pManager->ReleaseSelectQuery(pRes);
	}
	if (!bUpdate)
	{
		sprintf_s(szQuery, 2048, "INSERT INTO token_info(phone, token, fire, gas, spy, line, alert, hour, min, endhour, endmin, usetime, mobiletype) VALUES('%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)"
			, pReq->szPhoneNo, pReq->szToken, pReq->nFire, pReq->nGas, pReq->nSpy, pReq->nLine, pReq->nAlert, pReq->nHour, pReq->nMin, pReq->nEndHour, pReq->nEndMin, pReq->nUseTime, pReq->nMobileType);
		pManager->MysqlExcuteQuery(szQuery);
	}
	else
	{
		sprintf_s(szQuery, 2048, "UPDATE token_info set token='%s', fire=%d, gas=%d, spy=%d, line=%d, alert=%d, hour=%d, min=%d, endhour=%d, endmin=%d, usetime=%d, mobiletype=%d WHERE phone='%s';"
			, pReq->szToken, pReq->nFire, pReq->nGas, pReq->nSpy, pReq->nLine, pReq->nAlert, pReq->nHour, pReq->nMin, pReq->nEndHour, pReq->nEndMin, pReq->nUseTime, pReq->nMobileType, pReq->szPhoneNo);
		pManager->MysqlExcuteQuery(szQuery);
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	trace0("ProcessProtocolRequestSetToken- phone no: %s, token: %s", pReq->szPhoneNo, pReq->szToken);

	// dont send response
}

void Server::GetEventCount(int & nFire, int & nGas, int & nSpy, int & nLine, char* szSeq)
{
	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;
	char szTemp[16], szTemp1[16];

	nFire = 0, nGas = 0, nSpy = 0, nLine = 0;
	int nSeq = 0, nRecovery = 0, nCount = 0;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT seq FROM event_log WHERE EVENT LIKE \"Rx%%\" AND seq>0 AND manager_seq IN(%s) order by seq DESC LIMIT 1"
		, szSeq);
	pRes = pManager->MysqlSelectQuery(szQuery);
	if (pRes)
	{
		if (pRes && mysql_num_rows(pRes) > 0)
		{
			if (ContentRow = mysql_fetch_row(pRes))
			{
				nSeq = atoi((char*)ContentRow[0]);
			}
		}
		pManager->ReleaseSelectQuery(pRes);
	}

	for (int nIndex = 0; nIndex < 4; nIndex++)
	{
		switch (nIndex)
		{
		case 0: strcpy_s(szTemp, "'Dx00%%'"); strcpy_s(szTemp1, "'Ax00%%'"); break;
		case 1: strcpy_s(szTemp, "'Dx01%%'"); strcpy_s(szTemp1, "'Ax01%%'"); break;
		case 2: strcpy_s(szTemp, "'Dx02%%'"); strcpy_s(szTemp1, "'Ax02%%'"); break;
		case 3: strcpy_s(szTemp, "'Dx03%%'"); strcpy_s(szTemp1, "'Ax03%%'"); break;
		}
		// 화재
		sprintf_s(szQuery, 2048, "SELECT a.aa, b.bb FROM (SELECT count(seq) aa \
		FROM event_log WHERE EVENT LIKE %s AND seq>%d AND manager_seq IN(%s)) a, \
		(SELECT count(seq) bb FROM event_log WHERE EVENT LIKE %s AND seq>%d AND manager_seq IN(%s)) b"
			, szTemp, nSeq, szSeq, szTemp1, nSeq, szSeq);
		pRes = pManager->MysqlSelectQuery(szQuery);
		if (pRes)
		{
			if (pRes && mysql_num_rows(pRes) > 0)
			{
				if (ContentRow = mysql_fetch_row(pRes))
				{
					nRecovery = atoi((char*)ContentRow[0]);
					nCount = atoi((char*)ContentRow[1]);
				}
			}
			pManager->ReleaseSelectQuery(pRes);
		}
		switch (nIndex)
		{
		case 0: (nRecovery > nCount) ? nFire = 0 : nFire = nCount - nRecovery; break;
		case 1: (nRecovery > nCount) ? nGas = 0 : nGas = nCount - nRecovery; break;
		case 2: (nRecovery > nCount) ? nSpy = 0 : nSpy = nCount - nRecovery; break;
		case 3: (nRecovery > nCount) ? nLine = 0 : nLine = nCount - nRecovery; break;
		}
	}

	CDBPool::Instance()->ReturnDBManager(pManager);
}

void Server::ProcessProtocolRequestGetEventList(BYTE* pData, Packet* packet)	// 일련번호 이후의 이벤트 리스트 요청
{
	ProtocolRequestGetEventList* pReq = (ProtocolRequestGetEventList*)pData;
	ProtocolResponseGetEventList* pEventList = NULL;

	char szSeq[32];
	//int nUserIndex = 0;
	int nFire = 0, nGas = 0, nSpy = 0, nLine = 0;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT manager_seq FROM user_account WHERE mobile='%s' and isuse='1'", pReq->szPhoneNo);
	pRes = pManager->MysqlSelectQuery(szQuery);
	CUIntArray arrayList;
	if (pRes)
	{
		if (pRes && mysql_num_rows(pRes) > 0)
		{
			while (ContentRow = mysql_fetch_row(pRes))
			{
				arrayList.Add(atoi((char*)ContentRow[0]));
				//nUserIndex = atoi((char*)ContentRow[0]);
			}
		}
		pManager->ReleaseSelectQuery(pRes);
	}

	if (arrayList.GetCount() == 0)
	{
		sprintf_s(szQuery, 2048, "SELECT SEQ, EVENT FROM event_log WHERE manager_seq=%d AND seq>%d order by seq DESC LIMIT 20"
			, 0, pReq->nSequenceNo);
	}
	else {
		char szKey[32];
		char szTemp[32];
		szKey[0] = '\0';
		for (int ii = 0; ii < arrayList.GetCount(); ii++) {
			sprintf_s(szTemp, "%d", arrayList.GetAt(ii));
			strcat_s(szKey, szTemp);
			if (ii + 1 < arrayList.GetCount()) {
				strcat_s(szKey, ",");
			}
		}
		// counting
		GetEventCount(nFire, nGas, nSpy, nLine, szKey);

		if (pReq->nType == 0) // 신규
		{
			sprintf_s(szTemp, "seq>%d", pReq->nSequenceNo);
		}
		else // 기존 이하
		{
			sprintf_s(szTemp, "seq<%d", pReq->nSequenceNo);
		}
		sprintf_s(szQuery, 2048, "SELECT SEQ, EVENT FROM event_log WHERE %s AND manager_seq IN(", szTemp);
		strcat_s(szQuery, szKey);
		if (pReq->nType == 0) // 신규
		{
			strcat_s(szQuery, ") order by seq DESC LIMIT 20");
		}
		else // 기존 이하
		{
			strcat_s(szQuery, ") order by seq DESC LIMIT 10");
		}
	}
	pRes = pManager->MysqlSelectQuery(szQuery);

	int nSize = sizeof(ProtocolResponseGetEventList);
	// 존재 하면 성공
	int nRowCount = 0;
	int nTerm = 256;
	int nPoint = 0;
	int nLen = 0;
	if (pRes)
	{
		nRowCount = mysql_num_rows(pRes);
		if (pRes && nRowCount > 0)
		{
			TRACE("success\n");
			nSize = sizeof(ProtocolResponseGetEventList) + (nTerm + 1) * nRowCount;
			pEventList = (ProtocolResponseGetEventList*) new BYTE[nSize];
			memset(pEventList, 0x00, nSize);
			bool bFirst = true;
			while (ContentRow = mysql_fetch_row(pRes))
			{
				if (!bFirst)
				{
					pEventList->szEventList[nPoint++] = ';';
				}
				bFirst = false;

				// sequence
				sprintf_s(szSeq, "%d;", atoi((char*)ContentRow[0]));
				strcat(&pEventList->szEventList[nPoint], szSeq);
				nLen = strlen(szSeq);
				nPoint += nLen;
				// event list
				strcat(&pEventList->szEventList[nPoint], (char*)ContentRow[1]);
				nLen = strlen((char*)ContentRow[1]);
				nPoint += nLen;
			}
		}
		pManager->ReleaseSelectQuery(pRes);
	}
	else
	{
		nSize = sizeof(ProtocolResponseGetEventList) + (nTerm + 1) * nRowCount;
		pEventList = (ProtocolResponseGetEventList*) new BYTE[nSize];
		memset(pEventList, 0x00, nSize);
	}

	CDBPool::Instance()->ReturnDBManager(pManager);

	pEventList->nCount = nRowCount;
	pEventList->nType = pReq->nType;
	pEventList->nFire = nFire;
	pEventList->nGas = nGas;
	pEventList->nSpy = nSpy;
	pEventList->nLine = nLine;
	pEventList->size = sizeof(ProtocolResponseGetEventList) + nPoint;
	trace0("ProcessProtocolRequestGetEventList- user: %s, seq: %d, count: %d, type: %d", pReq->szPhoneNo, pReq->nSequenceNo, nRowCount, pReq->nType);

	Server::Instance()->Send(packet, (BYTE*)pEventList, sizeof(ProtocolResponseGetEventList) + nPoint);

	SAFE_DELETE(pEventList);
}

void Server::ProcessProtocolRequestAdminLogin(BYTE* pData, Packet* packet) // 관리자 로그인
{
	CEncDec::Instance()->PacketDecode(pData, sizeof(ProtocolHeader), sizeof(ProtocolRequestLogin) - sizeof(ProtocolHeader));
	ProtocolRequestLogin* pReq = (ProtocolRequestLogin*)pData;
	ProtocolResponse res;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;

	sprintf_s(szQuery, 2048, "select idx from admin_user where id='%s' and pw=PASSWORD('%s')", pReq->szID, pReq->szPW);
	pRes = pManager->MysqlSelectQuery(szQuery);

	// 존재 하면 성공
	if (pRes && (UINT)mysql_num_rows(pRes) == 1)
	{
		TRACE("success\n");
		while (ContentRow = mysql_fetch_row(pRes))
		{
			res.nResult = CCommonFunc::GetAtoUI((char*)ContentRow[0]);
		}
		Log::WriteLog(0, szQuery);
	}
	else
	{
		res.nResult = 0;
		Log::WriteLog(0, "query: %s, get db failed", szQuery);
	}
	pManager->ReleaseSelectQuery(pRes);

	CDBPool::Instance()->ReturnDBManager(pManager);

	res.protocol = ProtocolHeader::ResponseAdminLogin;

	trace0("ProcessProtocolRequestAdminLogin- user: %s, result: %d", pReq->szID, res.nResult);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
}

void Server::ProcessProtocolRequestAddNewSystem(BYTE* pData, Packet* packet)	// 건물, 단지 등 시스템 추가
{
	ProtocolRequestAddNewSystem* pReq = (ProtocolRequestAddNewSystem*)pData;
	ProtocolResponse res;
	res.protocol = ProtocolHeader::ResponseAddNewSystem;
	res.size = sizeof(ProtocolResponse);

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT seq FROM work_site WHERE isuse=1 and name='%s'", pReq->szSystemName);
	pRes = pManager->MysqlSelectQuery(szQuery);
	if (pRes)
	{
		if (pRes && mysql_num_rows(pRes) > 0)
		{
			pManager->ReleaseSelectQuery(pRes);

			res.nResult = -1; // exist
			Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));

			CDBPool::Instance()->ReturnDBManager(pManager);
			return;
		}
		pManager->ReleaseSelectQuery(pRes);
	}

	sprintf_s(szQuery, 2048, "INSERT INTO work_site(name, location, etc) VALUES('%s', '%s', '%s')", pReq->szSystemName, pReq->szLocation, pReq->szEtc);
	if (pManager->MysqlExcuteQuery(szQuery))
	{
		res.nResult = 0; // success
	}
	else
	{
		res.nResult = -2; // fail
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
	trace0("ProcessProtocolRequestAddNewSystem - name: %s", pReq->szSystemName);
}

void Server::ProcessProtocolRequestModSystem(BYTE* pData, Packet* packet)		// 수정
{
	ProtocolRequestModSystem* pReq = (ProtocolRequestModSystem*)pData;
	ProtocolResponse res;
	res.protocol = ProtocolHeader::ResponseModSystem;
	res.size = sizeof(ProtocolResponse);

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT seq FROM work_site WHERE isuse=1 and name='%s' and seq<>%d", pReq->szSystemName, pReq->nSequenceNo);
	pRes = pManager->MysqlSelectQuery(szQuery);
	if (pRes)
	{
		if (pRes && mysql_num_rows(pRes) > 0)
		{
			pManager->ReleaseSelectQuery(pRes);

			res.nResult = -1; // exist
			Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
			trace0("ProcessProtocolRequestModSystem - exist system name: %s", pReq->szSystemName);

			CDBPool::Instance()->ReturnDBManager(pManager);
			return;
		}
		pManager->ReleaseSelectQuery(pRes);
	}

	sprintf_s(szQuery, 2048, "UPDATE work_site SET name='%s', location='%s', etc='%s' WHERE seq=%d"
		, pReq->szSystemName, pReq->szLocation, pReq->szEtc, pReq->nSequenceNo);

	if (pManager->MysqlExcuteQuery(szQuery))
	{
		res.nResult = 0; // success
	}
	else
	{
		res.nResult = -2; // fail
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
	trace0("ProcessProtocolRequestModSystem - name: %s, location: %s, etc: %s", pReq->szSystemName, pReq->szLocation, pReq->szEtc);
}

void Server::ProcessProtocolRequestDelSystem(BYTE* pData, Packet* packet)		// 삭제
{
	ProtocolRequestDelSystem* pReq = (ProtocolRequestDelSystem*)pData;
	ProtocolResponse res;
	res.protocol = ProtocolHeader::ResponseDelSystem;
	res.size = sizeof(ProtocolResponse);

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;

	CUIntArray arrayList;
	sprintf_s(szQuery, 2048, "SELECT seq FROM manager_account WHERE isuse='1' and worksite_seq=%d", pReq->nSequenceNo);
	pRes = pManager->MysqlSelectQuery(szQuery);
	if (pRes) {
		int nRowCount = mysql_num_rows(pRes);
		if (nRowCount > 0) {
			while (ContentRow = mysql_fetch_row(pRes))
			{
				arrayList.Add(atoi(ContentRow[0]));
			}
		}
		pManager->ReleaseSelectQuery(pRes);
	}
	for (int i = 0; i < arrayList.GetCount(); i++) {
		sprintf_s(szQuery, 2048, "UPDATE user_account SET isuse=0 WHERE manager_seq=%d", arrayList.GetAt(i));
		pManager->MysqlExcuteQuery(szQuery);
	}
	sprintf_s(szQuery, 2048, "UPDATE manager_account SET isuse='0' WHERE worksite_seq=%d", pReq->nSequenceNo);
	pManager->MysqlExcuteQuery(szQuery);

	sprintf_s(szQuery, 2048, "UPDATE work_site SET isuse='0' WHERE seq=%d", pReq->nSequenceNo);

	if (pManager->MysqlExcuteQuery(szQuery))
	{
		res.nResult = 1; // success
	}
	else
	{
		res.nResult = 0; // fail
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
	trace0("ProcessProtocolRequestDelSystem - sequence: %d", pReq->nSequenceNo);
}

void Server::ProcessProtocolRequestGetSystemList(BYTE* pData, Packet* packet)	// 시스템 목록 요청
{
	ProtocolRequest* pReq = (ProtocolRequest*)pData;
	ProtocolResponseGetSystemList* pResult = NULL;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT seq, name, location, etc, regdate FROM work_site WHERE isuse='1'");
	pRes = pManager->MysqlSelectQuery(szQuery);
	if (!pRes)
	{
		ProtocolResponseGetSystemList res;
		res.nCount = 0;

		CDBPool::Instance()->ReturnDBManager(pManager);

		Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponseGetSystemList));
		trace0("ProcessProtocolRequestGetSystemList - count: 0");
		return;
	}
	int nSize = sizeof(ProtocolResponseGetSystemList);
	int nRowCount = 0;
	if (pRes) {
		nRowCount = mysql_num_rows(pRes);
	}

	if (pRes && nRowCount > 0)
	{
		nSize = sizeof(ProtocolResponseGetSystemList) + 600 * nRowCount;
		pResult = (ProtocolResponseGetSystemList*) new BYTE[nSize];
		memset(pResult, 0x00, nSize);

		/*ContentRow = mysql_fetch_row(pRes);
		while (nTest++ < 10000)*/
		int nIndex = 0;
		while (ContentRow = mysql_fetch_row(pRes))
		{
			strcat(pResult->szSystemInfo, (char*)ContentRow[0]);
			strcat(pResult->szSystemInfo, ";");
			strcat(pResult->szSystemInfo, (char*)ContentRow[1]);
			strcat(pResult->szSystemInfo, ";");
			strcat(pResult->szSystemInfo, (char*)ContentRow[2]);
			strcat(pResult->szSystemInfo, ";");
			strcat(pResult->szSystemInfo, (char*)ContentRow[3]);
			strcat(pResult->szSystemInfo, ";");
			strcat(pResult->szSystemInfo, (char*)ContentRow[4]);
			strcat(pResult->szSystemInfo, ";");
		}
		nSize = sizeof(ProtocolResponseGetSystemList) + strlen(pResult->szSystemInfo) + 1;

		pResult->protocol = ProtocolHeader::ResponseGetSystemList;
		pResult->size = nSize;
		pResult->nCount = nRowCount;
	}
	else {
		pResult = new ProtocolResponseGetSystemList();
		pResult->size = sizeof(ProtocolResponseGetSystemList);
		pResult->nCount = 0;
		nSize = sizeof(ProtocolResponseGetSystemList);
	}
	pManager->ReleaseSelectQuery(pRes);

	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)pResult, nSize);
	trace0("ProcessProtocolRequestGetSystemList - count: %d", pResult->nCount);

	SAFE_DELETE(pResult);
}

// manager
void Server::ProcessProtocolRequestGetManagerList(BYTE* pData, Packet* packet)
{
	ProtocolRequest* pReq = (ProtocolRequest*)pData;
	ProtocolResponseGetManagerList* pResult = NULL;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT seq, worksite_seq, user_limit, id, etc, regdate FROM manager_account WHERE worksite_seq=%d AND isuse='1'"
	, pReq->nValue);
	pRes = pManager->MysqlSelectQuery(szQuery);

	int nSize = sizeof(ProtocolResponseGetManagerList);
	int nRowCount = 0;

	if (pRes)
	{
		nRowCount = mysql_num_rows(pRes);

		nSize = sizeof(ProtocolResponseGetManagerList) + sizeof(ProtocolResponseGetManagerList::managerInfo) * nRowCount;
		pResult = (ProtocolResponseGetManagerList*) new BYTE[nSize];
		memset(pResult, 0x00, nSize);

		int nIndex = 0;
		while (ContentRow = mysql_fetch_row(pRes))
		{
			pResult->info[nIndex].nSeq = atoi(ContentRow[0]);
			pResult->info[nIndex].nWorksiteSeq = atoi(ContentRow[1]);
			pResult->info[nIndex].nUserLimit = atoi(ContentRow[2]);
			strcat(pResult->info[nIndex].szID, (char*)ContentRow[3]);
			strcat(pResult->info[nIndex].szEtc, (char*)ContentRow[4]);
			strcat(pResult->info[nIndex].szRegdate, (char*)ContentRow[5]);

			nIndex++;
		}

		pResult->protocol = ProtocolHeader::ResponseGetManagerList;
		pResult->size = nSize;
		pResult->nCount = nRowCount;
	}
	else
	{
		ProtocolResponseGetManagerList res;
		res.nCount = 0;

		CDBPool::Instance()->ReturnDBManager(pManager);

		Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponseGetManagerList));
		trace0("ProcessProtocolResponseGetManagerList - count: 0");
		return;
	}
	pManager->ReleaseSelectQuery(pRes);

	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)pResult, nSize);
	trace0("ProcessProtocolRequestGetManagerList - count: %d", pResult->nCount);

	SAFE_DELETE(pResult);
}

void Server::ProcessProtocolRequestAddManager(BYTE* pData, Packet* packet)
{
	ProtocolRequestAddManager* pReq = (ProtocolRequestAddManager*)pData;
	ProtocolResponse res;
	res.protocol = ProtocolHeader::ResponseAddManager;
	res.size = sizeof(ProtocolResponse);

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT seq FROM manager_account WHERE id='%s' and pw=PASSWORD('%s') and isuse=1", pReq->szID, pReq->szPW);
	pRes = pManager->MysqlSelectQuery(szQuery);
	if (pRes)
	{
		if (pRes && mysql_num_rows(pRes) > 0)
		{
			pManager->ReleaseSelectQuery(pRes);

			res.nResult = -1; // exist
			Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
			trace0("ProcessProtocolRequestAddManager - exist manager id: %s", pReq->szID);

			CDBPool::Instance()->ReturnDBManager(pManager);
			return;
		}
		pManager->ReleaseSelectQuery(pRes);
	}

	sprintf_s(szQuery, 2048, "INSERT INTO manager_account(worksite_seq, user_limit, id, pw, etc) VALUES(%d, %d, '%s', PASSWORD('%s'), '%s')"
		, pReq->nWorksiteSeq, pReq->nUserLimit, pReq->szID, pReq->szPW, pReq->szEtc);
	if (pManager->MysqlExcuteQuery(szQuery))
	{
		res.nResult = 0; // success
	}
	else
	{
		res.nResult = -2; // fail
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
	trace0("ProcessProtocolRequestAddManager - id: %s", pReq->szID);
}

void Server::ProcessProtocolRequestModManager(BYTE* pData, Packet* packet)
{
	ProtocolRequestModManager* pReq = (ProtocolRequestModManager*)pData;
	ProtocolResponse res;
	res.protocol = ProtocolHeader::ResponseModManager;
	res.size = sizeof(ProtocolResponse);

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT seq FROM manager_account WHERE id='%s' and pw=PASSWORD('%s') and id<>'%d' and isuse=1", pReq->szID, pReq->szPW, pReq->nSeq);
	pRes = pManager->MysqlSelectQuery(szQuery);
	if (pRes)
	{
		if (pRes && mysql_num_rows(pRes) > 0)
		{
			pManager->ReleaseSelectQuery(pRes);

			res.nResult = -1; // exist
			Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
			trace0("ProcessProtocolRequestModManager - exist manager id: %s", pReq->szID);

			CDBPool::Instance()->ReturnDBManager(pManager);
			return;
		}
		pManager->ReleaseSelectQuery(pRes);
	}

	if (strlen(pReq->szPW) > 0) {
		sprintf_s(szQuery, 2048, "UPDATE manager_account SET pw=PASSWORD('%s'), user_limit=%d, etc='%s' WHERE seq=%d"
			, pReq->szPW, pReq->nUserLimit, pReq->szEtc, pReq->nSeq);
	}
	else {
		sprintf_s(szQuery, 2048, "UPDATE manager_account SET user_limit=%d, etc='%s' WHERE seq=%d"
			, pReq->nUserLimit, pReq->szEtc, pReq->nSeq);
	}

	if (pManager->MysqlExcuteQuery(szQuery))
	{
		res.nResult = 1; // success
	}
	else
	{
		res.nResult = 0; // fail
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
	trace0("ProcessProtocolRequestModManager - limit: %d, etc: %s, seq: %d", pReq->nUserLimit, pReq->szEtc, pReq->nSeq);
}

void Server::ProcessProtocolRequestDelManager(BYTE* pData, Packet* packet)
{
	ProtocolRequestDelManager* pReq = (ProtocolRequestDelManager*)pData;
	ProtocolResponse res;
	res.protocol = ProtocolHeader::ResponseDelManager;
	res.size = sizeof(ProtocolResponse);

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "UPDATE user_account SET isuse=0 WHERE manager_seq=%d", pReq->nSeq);
	pManager->MysqlExcuteQuery(szQuery);

	sprintf_s(szQuery, 2048, "UPDATE manager_account SET isuse='0' WHERE seq=%d", pReq->nSeq);

	if (pManager->MysqlExcuteQuery(szQuery))
	{
		res.nResult = 1; // success
	}
	else
	{
		res.nResult = 0; // fail
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
	trace0("ProcessProtocolRequestDelManager - seq: %d", pReq->nSeq);
}

// user
void Server::ProcessProtocolRequestGetUserList(BYTE* pData, Packet* packet)
{
	ProtocolRequest* pReq = (ProtocolRequest*)pData;
	BYTE* pResult = NULL;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT seq, name, mobile, etc FROM user_account WHERE manager_seq=%d AND isuse='1'", pReq->nValue);
	pRes = pManager->MysqlSelectQuery(szQuery);
	int nCount = 0;
	int nSize = 0;
	if (pRes)
	{
		nCount = mysql_num_rows(pRes);
		nSize = sizeof(ProtocolResponseGetUserList) + sizeof(ProtocolResponseGetUserList::userInfo) * nCount;
		pResult = new BYTE[nSize];
		((ProtocolResponseGetUserList*)pResult)->nCount = nCount;
		((ProtocolResponseGetUserList*)pResult)->protocol = ProtocolHeader::ResponseGetUserList;
		((ProtocolResponseGetUserList*)pResult)->size = nSize;
		int nIndex = 0;
		while (ContentRow = mysql_fetch_row(pRes))
		{
			((ProtocolResponseGetUserList*)pResult)->info[nIndex].nSeq = atoi((char*)ContentRow[0]);
			strcpy_s(((ProtocolResponseGetUserList*)pResult)->info[nIndex].szName, (char*)ContentRow[1]);
			strcpy_s(((ProtocolResponseGetUserList*)pResult)->info[nIndex].szMobile, (char*)ContentRow[2]);
			strcpy_s(((ProtocolResponseGetUserList*)pResult)->info[nIndex].szEtc, (char*)ContentRow[3]);
			++nIndex;
		}
		pManager->ReleaseSelectQuery(pRes);
	}
	else
	{
		nSize = sizeof(ProtocolResponseGetUserList);
		pResult = new BYTE[nSize];
		((ProtocolResponseGetUserList*)pResult)->nCount = 0;
		((ProtocolResponseGetUserList*)pResult)->protocol = ProtocolHeader::ResponseGetUserList;
		((ProtocolResponseGetUserList*)pResult)->size = nSize;
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	trace0("ProcessProtocolRequestGetUserList- manager: %d", pReq->nValue);

	Server::Instance()->Send(packet, (BYTE*)pResult, nSize);

	SAFE_DELETE(pResult);
}

void Server::ProcessProtocolRequestGetUserTokenList(BYTE* pData, Packet* packet)	// 해당 시스템에 등록된 유저 목록 요청
{
	ProtocolRequest* pReq = (ProtocolRequest*)pData;
	BYTE* pResult = NULL;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;

	/*sprintf_s(szQuery, 2048, "INSERT INTO token_info(phone, token, fire, gas, spy, line, alert, hour, min, endhour, endmin, usetime, mobiletype) VALUES('%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)"
		, pReq->szPhoneNo, pReq->szToken, pReq->nFire, pReq->nGas, pReq->nSpy, pReq->nLine, pReq->nAlert, pReq->nHour, pReq->nMin, pReq->nEndHour, pReq->nEndMin, pReq->nUseTime, pReq->nMobileType);*/

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT b.phone, b.token, b.fire, b.gas, b.spy, b.line, b.alert, b.hour, b.min, b.endhour, b.endmin, b.usetime, b.mobiletype FROM user_account a, token_info b WHERE a.manager_seq=%d AND a.isuse='1' AND a.mobile=b.phone"
		, pReq->nValue);
	pRes = pManager->MysqlSelectQuery(szQuery);
	int nCount = 0;
	int nSize = 0;
	if (pRes)
	{
		nCount = mysql_num_rows(pRes);
		nSize = sizeof(ProtocolResponseGetUserTokenList) + sizeof(ProtocolResponseGetUserTokenList::userInfo) * nCount;
		pResult = new BYTE[nSize];
		((ProtocolResponseGetUserTokenList*)pResult)->nCount = nCount;
		((ProtocolResponseGetUserTokenList*)pResult)->protocol = ProtocolHeader::ResponseGetUserTokenList;
		((ProtocolResponseGetUserTokenList*)pResult)->size = nSize;
		int nIndex = 0;
		while (ContentRow = mysql_fetch_row(pRes))
		{
			strcpy_s(((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].szPhoneNo, (char*)ContentRow[0]);
			strcpy_s(((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].szToken, (char*)ContentRow[1]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nFire = atoi(ContentRow[2]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nGas = atoi(ContentRow[3]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nSpy = atoi(ContentRow[4]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nLine = atoi(ContentRow[5]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nAlert = atoi(ContentRow[6]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nHour = atoi(ContentRow[7]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nMin = atoi(ContentRow[8]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nEndHour = atoi(ContentRow[9]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nEndMin = atoi(ContentRow[10]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nUseTime = atoi(ContentRow[11]);
			((ProtocolResponseGetUserTokenList*)pResult)->info[nIndex].nMobileType = atoi(ContentRow[12]);
			++nIndex;
		}
		pManager->ReleaseSelectQuery(pRes);
	}
	else
	{
		nSize = sizeof(ProtocolResponseGetUserTokenList);
		pResult = new BYTE[nSize];
		((ProtocolResponseGetUserTokenList*)pResult)->nCount = 0;
		((ProtocolResponseGetUserTokenList*)pResult)->protocol = ProtocolHeader::ResponseGetUserTokenList;
		((ProtocolResponseGetUserTokenList*)pResult)->size = nSize;
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	trace0("ProcessProtocolRequestGetUserTokenList- manager: %d", pReq->nValue);

	Server::Instance()->Send(packet, (BYTE*)pResult, nSize);

	SAFE_DELETE(pResult);
}

void Server::ProcessProtocolRequestAddUser(BYTE* pData, Packet* packet)		// 유저 추가
{
	ProtocolRequestAddUser* pReq = (ProtocolRequestAddUser*)pData;
	ProtocolResponse res;
	res.protocol = ProtocolHeader::ResponseAddUser;
	res.size = sizeof(ProtocolResponse);

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT seq FROM user_account WHERE isuse=1 and mobile='%s' and manager_seq=%d", pReq->szMobile, pReq->nManagerSeq);
	pRes = pManager->MysqlSelectQuery(szQuery);
	if (pRes)
	{
		if (pRes && mysql_num_rows(pRes) > 0)
		{
			pManager->ReleaseSelectQuery(pRes);

			res.nResult = -1; // exist
			Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
			trace0("ProcessProtocolRequestAddUser - exist mobile no: %s", pReq->szMobile);

			CDBPool::Instance()->ReturnDBManager(pManager);
			return;
		}
		pManager->ReleaseSelectQuery(pRes);
	}

	sprintf_s(szQuery, 2048, "INSERT INTO user_account(manager_seq, name, mobile, etc) VALUES(%d, '%s', '%s', '%s')",
		pReq->nManagerSeq, pReq->szName, pReq->szMobile, pReq->szEtc);
	if (pManager->MysqlExcuteQuery(szQuery))
	{
		res.nResult = 0; // success
	}
	else
	{
		res.nResult = -2; // fail
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
	trace0("ProcessProtocolRequestAddUser - name: %s, mobile: %s", pReq->szName, pReq->szMobile);
}

void Server::ProcessProtocolRequestModUser(BYTE* pData, Packet* packet)		// 유저 수정
{
	ProtocolRequestModUser* pReq = (ProtocolRequestModUser*)pData;
	ProtocolResponse res;
	res.protocol = ProtocolHeader::ResponseModUser;
	res.size = sizeof(ProtocolResponse);

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "UPDATE user_account SET name='%s', mobile='%s', etc='%s' WHERE seq=%d"
		, pReq->szName, pReq->szMobile, pReq->szEtc, pReq->nSeq);

	if (pManager->MysqlExcuteQuery(szQuery))
	{
		res.nResult = 1; // success
	}
	else
	{
		res.nResult = 0; // fail
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
	trace0("ProcessProtocolRequestModUser - name: %s, mobile: %s, etc: %s, seq: %d"
		, pReq->szName, pReq->szMobile, pReq->szEtc, pReq->nSeq);
}

void Server::ProcessProtocolRequestDelUser(BYTE* pData, Packet* packet)		// 유저 삭제
{
	ProtocolRequestDelUser* pReq = (ProtocolRequestDelUser*)pData;
	ProtocolResponse res;
	res.protocol = ProtocolHeader::ResponseDelUser;
	res.size = sizeof(ProtocolResponse);

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "UPDATE user_account SET isuse='0' WHERE seq=%d", pReq->nSeq);

	if (pManager->MysqlExcuteQuery(szQuery))
	{
		res.nResult = 1; // success
	}
	else
	{
		res.nResult = 0; // fail
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
	trace0("ProcessProtocolRequestDelUser - seq: %d", pReq->nSeq);
}

void Server::ProcessProtocolRequestAddEvent(BYTE* pData, Packet* packet)
{
	ProtocolRequestAddEvent* pReq = (ProtocolRequestAddEvent*)pData;
	ProtocolResponse res;

	CHAR szQuery[2048];
	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;

	CStringA sTemp;
	sTemp = pReq->szEvent;
	sTemp.Replace("\r", "");
	sTemp.Replace("\n", "");

	CString sEvent;
	CStringA sLogEvent;
	CString sWValue = CCommonFunc::Utf8ToWCHAR(pReq->szEvent);
	int nCount = 0;
	while (AfxExtractSubString(sEvent, sWValue, nCount, L'§')) {
		sTemp = CCommonFunc::WCharToUtf8(sEvent.GetBuffer(0));
		if (sLogEvent.GetLength() == 0) {
			sLogEvent = CCommonFunc::WCharToChar(sEvent.GetBuffer(0));
		}
		sprintf_s(szQuery, 2048, "INSERT INTO event_log(manager_seq, EVENT) VALUES(%d, '%s')", pReq->idx, sTemp.GetBuffer(0));// pReq->szEvent);
		sTemp.ReleaseBuffer();
		if (pManager->MysqlExcuteQuery(szQuery))
		{
			res.nResult = 1;
		}
		else
		{
			res.nResult = 0;
		}
		++nCount;
	}
	CDBPool::Instance()->ReturnDBManager(pManager);

	res.protocol = ProtocolHeader::ResponseAddEvent;

	trace0("ProcessProtocolRequestAddEvent- count: %d, user: %d, event: %s", nCount, pReq->idx, sLogEvent.GetBuffer(0));

	Server::Instance()->Send(packet, (BYTE*)&res, sizeof(ProtocolResponse));
}

void Server::ProcessProtocolRequestSetToken1(BYTE* pData, Packet* packet)	// 토큰, 전화번호 전송
{
	BYTE pbszIV[] = { 0x26, 0x6d, 0x66, 0x47, 0x35, 0x18, 0x1a, 0x61, 0x6f, 0x0a, 0x39, 0x7a, 0x36, 0x16, 0x25, 0x01, 0x00 };

	BYTE info[322];
	ProtocolRequestSetToken* pReq = NULL;
	ProtocolRequestEncrypt* pReq1 = (ProtocolRequestEncrypt*)pData;
	BYTE data[320];
	SEED_CBC_Decrypt(pbszIV, pReq1->bVector, pReq1->bEncryptText, 320, data);

	memcpy_s(info, 6, pData, 6);
	memcpy_s(&info[6], 316, data, 316);
	pReq = (ProtocolRequestSetToken*)info;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	bool bUpdate = false;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "DELETE FROM token_info WHERE token='%s'", pReq->szToken);
	pRes = pManager->MysqlSelectQuery(szQuery);

	if (strcmp(pReq->szPhoneNo, "0") == 0)
	{
		CDBPool::Instance()->ReturnDBManager(pManager);
		return;
	}
	sprintf_s(szQuery, 2048, "DELETE FROM token_info WHERE phone='%s'", pReq->szPhoneNo);
	pRes = pManager->MysqlSelectQuery(szQuery);

	sprintf_s(szQuery, 2048, "INSERT INTO token_info(phone, token, fire, gas, spy, line, alert, hour, min, endhour, endmin, usetime, mobiletype) VALUES('%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)"
			, pReq->szPhoneNo, pReq->szToken, pReq->nFire, pReq->nGas, pReq->nSpy, pReq->nLine, pReq->nAlert, pReq->nHour, pReq->nMin, pReq->nEndHour, pReq->nEndMin, pReq->nUseTime, pReq->nMobileType);
	pManager->MysqlExcuteQuery(szQuery);

	CDBPool::Instance()->ReturnDBManager(pManager);

	//CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	//sprintf_s(szQuery, 2048, "SELECT token FROM token_info WHERE phone='%s'", pReq->szPhoneNo);
	//pRes = pManager->MysqlSelectQuery(szQuery);
	//if (pRes)
	//{
	//	if (pRes && mysql_num_rows(pRes) > 0)
	//	{
	//		/*if (ContentRow = mysql_fetch_row(pRes))
	//		{
	//		if (strcmp(pReq->szToken, (char*)ContentRow[0]) == 0)
	//		{
	//		pManager->ReleaseSelectQuery(pRes);
	//		CDBPool::Instance()->ReturnDBManager(pManager);
	//		return;
	//		}
	//		}*/
	//		bUpdate = true;
	//	}
	//	pManager->ReleaseSelectQuery(pRes);
	//}
	//if (!bUpdate)
	//{
	//	sprintf_s(szQuery, 2048, "INSERT INTO token_info(phone, token, fire, gas, spy, line, alert, hour, min, endhour, endmin, usetime, mobiletype) VALUES('%s', '%s', %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d)"
	//		, pReq->szPhoneNo, pReq->szToken, pReq->nFire, pReq->nGas, pReq->nSpy, pReq->nLine, pReq->nAlert, pReq->nHour, pReq->nMin, pReq->nEndHour, pReq->nEndMin, pReq->nUseTime, pReq->nMobileType);
	//	pManager->MysqlExcuteQuery(szQuery);
	//}
	//else
	//{
	//	sprintf_s(szQuery, 2048, "UPDATE token_info set token='%s', fire=%d, gas=%d, spy=%d, line=%d, alert=%d, hour=%d, min=%d, endhour=%d, endmin=%d, usetime=%d, mobiletype=%d WHERE phone='%s';"
	//		, pReq->szToken, pReq->nFire, pReq->nGas, pReq->nSpy, pReq->nLine, pReq->nAlert, pReq->nHour, pReq->nMin, pReq->nEndHour, pReq->nEndMin, pReq->nUseTime, pReq->nMobileType, pReq->szPhoneNo);
	//	pManager->MysqlExcuteQuery(szQuery);
	//}
	//CDBPool::Instance()->ReturnDBManager(pManager);

	trace0("ProcessProtocolRequestSetToken- phone no: %s, token: %s", pReq->szPhoneNo, pReq->szToken);

	// dont send response
}

void Server::ProcessProtocolRequestGetEventListEnc(BYTE* pData, Packet* packet)
{
	ProtocolResponseGetEventList* pEventList = NULL;

	BYTE pbszIV[] = { 0x26, 0x6d, 0x66, 0x47, 0x35, 0x18, 0x1a, 0x61, 0x6f, 0x0a, 0x39, 0x7a, 0x36, 0x16, 0x25, 0x01, 0x00 };

	BYTE info[322];
	ProtocolRequestGetEventList* pReq = NULL;
	ProtocolRequestEncrypt* pReq1 = (ProtocolRequestEncrypt*)pData;
	BYTE data[320];
	SEED_CBC_Decrypt(pbszIV, pReq1->bVector, pReq1->bEncryptText, 32, data);

	memcpy_s(info, 6, pData, 6);
	memcpy_s(&info[6], 316, data, 316);
	pReq = (ProtocolRequestGetEventList*)info;
	pReq->protocol = ProtocolHeader::ResponseGetEventList;
	pReq->size = sizeof(ProtocolResponseGetEventList);


	char szSeq[32];
	//int nUserIndex = 0;
	int nFire = 0, nGas = 0, nSpy = 0, nLine = 0;

	MYSQL_RES* pRes = NULL;
	CHAR szQuery[2048];
	MYSQL_ROW  ContentRow;

	CDBConnectionManager* pManager = CDBPool::Instance()->GetDbManager(); if (!pManager) return;
	sprintf_s(szQuery, 2048, "SELECT manager_seq FROM user_account WHERE mobile='%s' and isuse='1'", pReq->szPhoneNo);
	pRes = pManager->MysqlSelectQuery(szQuery);
	CUIntArray arrayList;
	if (pRes)
	{
		if (pRes && mysql_num_rows(pRes) > 0)
		{
			while (ContentRow = mysql_fetch_row(pRes))
			{
				arrayList.Add(atoi((char*)ContentRow[0]));
				//nUserIndex = atoi((char*)ContentRow[0]);
			}
		}
		pManager->ReleaseSelectQuery(pRes);
	}

	if (arrayList.GetCount() == 0)
	{
		sprintf_s(szQuery, 2048, "SELECT SEQ, EVENT FROM event_log WHERE manager_seq=%d AND seq>%d order by seq DESC LIMIT 20"
			, 0, pReq->nSequenceNo);
	}
	else {
		char szKey[32];
		char szTemp[32];
		szKey[0] = '\0';
		for (int ii = 0; ii < arrayList.GetCount(); ii++) {
			sprintf_s(szTemp, "%d", arrayList.GetAt(ii));
			strcat_s(szKey, szTemp);
			if (ii + 1 < arrayList.GetCount()) {
				strcat_s(szKey, ",");
			}
		}
		// counting
		GetEventCount(nFire, nGas, nSpy, nLine, szKey);

		if (pReq->nType == 0) // 신규
		{
			sprintf_s(szTemp, "seq>%d", pReq->nSequenceNo);
		}
		else // 기존 이하
		{
			sprintf_s(szTemp, "seq<%d", pReq->nSequenceNo);
		}
		sprintf_s(szQuery, 2048, "SELECT SEQ, EVENT FROM event_log WHERE %s AND manager_seq IN(", szTemp);
		strcat_s(szQuery, szKey);
		if (pReq->nType == 0) // 신규
		{
			strcat_s(szQuery, ") order by seq DESC LIMIT 20");
		}
		else // 기존 이하
		{
			strcat_s(szQuery, ") order by seq DESC LIMIT 10");
		}
	}
	pRes = pManager->MysqlSelectQuery(szQuery);

	int nSize = sizeof(ProtocolResponseGetEventList);
	// 존재 하면 성공
	int nRowCount = 0;
	int nTerm = 256;
	int nPoint = 0;
	int nLen = 0;
	if (pRes)
	{
		nRowCount = mysql_num_rows(pRes);
		if (pRes && nRowCount > 0)
		{
			TRACE("success\n");
			nSize = sizeof(ProtocolResponseGetEventList) + (nTerm + 1) * nRowCount;
			pEventList = (ProtocolResponseGetEventList*) new BYTE[nSize];
			memset(pEventList, 0x00, nSize);
			bool bFirst = true;
			while (ContentRow = mysql_fetch_row(pRes))
			{
				if (!bFirst)
				{
					pEventList->szEventList[nPoint++] = ';';
				}
				bFirst = false;

				// sequence
				sprintf_s(szSeq, "%d;", atoi((char*)ContentRow[0]));
				strcat(&pEventList->szEventList[nPoint], szSeq);
				nLen = strlen(szSeq);
				nPoint += nLen;
				// event list
				strcat(&pEventList->szEventList[nPoint], (char*)ContentRow[1]);
				nLen = strlen((char*)ContentRow[1]);
				nPoint += nLen;
			}
		}
		pManager->ReleaseSelectQuery(pRes);
	}
	else
	{
		nSize = sizeof(ProtocolResponseGetEventList) + (nTerm + 1) * nRowCount;
		pEventList = (ProtocolResponseGetEventList*) new BYTE[nSize];
		memset(pEventList, 0x00, nSize);
	}

	CDBPool::Instance()->ReturnDBManager(pManager);

	pEventList->nCount = nRowCount;
	pEventList->nType = pReq->nType;
	pEventList->nFire = nFire;
	pEventList->nGas = nGas;
	pEventList->nSpy = nSpy;
	pEventList->nLine = nLine;
	pEventList->size = sizeof(ProtocolResponseGetEventList) + nPoint;
	trace0("ProcessProtocolRequestGetEventList- user: %s, seq: %d, count: %d, type: %d", pReq->szPhoneNo, pReq->nSequenceNo, nRowCount, pReq->nType);

	Server::Instance()->Send(packet, (BYTE*)pEventList, sizeof(ProtocolResponseGetEventList) + nPoint);

	SAFE_DELETE(pEventList);
}

#endif

void Server::ProcessGFSProtocolResponseError(int nResult, Packet* packet)
{
	GFSProtocolError gpe;
	gpe.result = nResult;

	Server::Instance()->Send(packet, (BYTE*)&gpe, sizeof(GFSProtocolError), GFSProtocolHeader::GFSProtocolCommand::ERROR_RESPONSE);
}

void Server::ProcessGFSProtocolRequestLogin(BYTE* pData, Packet* packet)
{
	GFSProtocolLoginRequest* pGplReq = (GFSProtocolLoginRequest*)pData;
	char szID[40];
	char szPW[40];
	memset(szID, NULL, 40);
	memset(szPW, NULL, 40);

// 	strcpy(szID, CCommonFunc::WCharToChar(CCommonFunc::Utf8ToWCHAR(pGplReq->szID)));
// 	strcpy(szPW, CCommonFunc::WCharToChar(CCommonFunc::Utf8ToWCHAR(pGplReq->szPW)));

	memcpy(szID, pGplReq->szID, 40);
	memcpy(szPW, pGplReq->szPW, 40);

	if (strcmp(szID, CCommonData::Instance()->m_szID) != 0)
	{
		ProcessGFSProtocolResponseError(GFSProtocolHeader::GFSProtocolResult::WRONG_LOGIN_ID, packet);
		//미로그인 상태
		return;
	}
	else if (strcmp(szPW, CCommonData::Instance()->m_szPW) != 0)
	{
		ProcessGFSProtocolResponseError(GFSProtocolHeader::GFSProtocolResult::WRONG_LOGIN_PW, packet);
		//미로그인 상태
		return;
	}

	//위에서 걸러지지 않으면 로그인 상태, 로그인 상태이어야만 keep alive, event 처리를 함
	GFSProtocolLoginResponse gplRes;
	gplRes.cmd = GFSProtocolHeader::GFSProtocolCommand::LOGIN_RESPONSE;
	Server::Instance()->SendAndSetLoggedInState(packet, (BYTE*)&gplRes, sizeof(GFSProtocolLoginResponse));
}

void Server::ProcessGFSProtocolRequestKeepAlive(BYTE* pData, Packet* packet)
{
	GFSProtocolKeepAliveRequest* pGpkaReq = (GFSProtocolKeepAliveRequest*)pData;

	GFSProtocolKeepAliveResponse gpkaRes;
	gpkaRes.cmd = GFSProtocolHeader::GFSProtocolCommand::KEEP_ALIVE_RESPONSE;
	Server::Instance()->Send(packet, (BYTE*)&gpkaRes, sizeof(GFSProtocolKeepAliveResponse), GFSProtocolHeader::GFSProtocolCommand::KEEP_ALIVE_RESPONSE);
}

void Server::ProcessGFSProtocolUnSolicitedEvent(BYTE* pData)
{
	CString strEventType = _T("");
	CString strOccurence = _T("");
	CString strMsg = _T("");

	GFSProtocolUnsolicitedEvent gpue;

	//20230817 GBM - 수신기 복구 시 감시 이벤트 수천건 발생으로 인해 불필요한 알람 방지를 위해 화재 및 수신기 복구 이벤트만 전송
	switch (pData[SI_EVENT_BUF_COMMAND])
	{
	case 'F':
	{
		gpue.eventType = GFSProtocolHeader::GFSProtocolEventType::FIRE_INFO;
		strEventType = _T("화재");
		break;
	}
	case 'G':
	{
		gpue.eventType = GFSProtocolHeader::GFSProtocolEventType::GAS_INFO;
		strEventType = _T("가스");
		//break;
		Log::Trace("가스 이벤트가 들어왔습니다. 가스 이벤트는 전송하지 않습니다.");
		return;
	}
	case 'S':
	{
		gpue.eventType = GFSProtocolHeader::GFSProtocolEventType::SURVEILLANCE_INFO;
		strEventType = _T("감시");
		//break;
		Log::Trace("감시 이벤트가 들어왔습니다. 감시 이벤트는 전송하지 않습니다.");
		return;
	}
	case 'T':
	{
		gpue.eventType = GFSProtocolHeader::GFSProtocolEventType::DISCONNECTION_INFO;
		strEventType = _T("단선");
		//break;
		Log::Trace("단선 이벤트가 들어왔습니다. 단선 이벤트는 전송하지 않습니다.");
		return;
	}
	case 'R':
	{
		gpue.eventType = GFSProtocolHeader::GFSProtocolEventType::RESTORATION_INFO;
		strEventType = _T("시스템 복구 신호");
		break;
	}
	default:
	{
		Log::Trace("이벤트 처리 중 잘못된 이벤트 타입이 들어와서 이벤트를 전송하지 않습니다.");
		return;
	}
	}

	switch (pData[SI_EVENT_BUF_OCCUR_INFO])
	{
	case 'N':
	{
		//발생이지만 이벤트 타입이 수신기 복구이면 일괄 해제로 넣음
		if (gpue.eventType != GFSProtocolHeader::GFSProtocolEventType::RESTORATION_INFO)
		{
			gpue.occurrence = GFSProtocolHeader::GFSProtocolOccurrence::OCCURRENCE;
			strOccurence = _T("발생");
		}
		else
		{
			gpue.occurrence = GFSProtocolHeader::GFSProtocolOccurrence::RESTORATION_ALL;
			strOccurence = _T("일괄 복구");
		}
		break;
	}
	case 'F':
	{
		gpue.occurrence = GFSProtocolHeader::GFSProtocolOccurrence::RESTORATION;
		strOccurence = _T("복구");
		break;
	}
	default:
	{
		Log::Trace("이벤트 처리 중 잘못된 발생 정보가 들어와서 이벤트를 전송하지 않습니다.");
		return;
	}
	}	

	CString strCircuitNo = _T("");
	strCircuitNo = CCircuitLocInfo::Instance()->GetCircuitNo(pData);
	
	std::map<CString, CIRCUIT_LOC_INFO>::iterator iter;
	
	//수신기 복구일 때는 회로 위치 정보가 없다.
	if (gpue.eventType != GFSProtocolHeader::GFSProtocolEventType::RESTORATION_INFO)
	{
		iter = CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.find(strCircuitNo);
		if (iter != CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.end())
		{
			strcpy_s(gpue.building, CCommonFunc::WcharToUtf8(CCommonFunc::CharToWCHAR(iter->second.buildingName)));
			strcpy_s(gpue.stair, CCommonFunc::WcharToUtf8(CCommonFunc::CharToWCHAR(iter->second.stair)));
			strcpy_s(gpue.floor, CCommonFunc::WcharToUtf8(CCommonFunc::CharToWCHAR(iter->second.floor)));
			strcpy_s(gpue.room, CCommonFunc::WcharToUtf8(CCommonFunc::CharToWCHAR(iter->second.room)));
			strcpy_s(gpue.circuit, CCommonFunc::WcharToUtf8(CCommonFunc::CharToWCHAR(iter->second.circuitName)));

// 			strcpy(gpue.building, iter->second.buildingName);
// 			strcpy(gpue.stair, iter->second.stair);
// 			strcpy(gpue.floor, iter->second.floor);
// 			strcpy(gpue.room, iter->second.room);
// 			strcpy(gpue.circuit, iter->second.circuitName);

			gpue.reserved = 0;

			CString strBuildingName = _T("");
			CString strStair = _T("");
			CString strFloor = _T("");
			CString strRoom = _T("");
			CString strCircuit = _T("");

			strBuildingName.Format(_T("%s"), CCommonFunc::CharToWCHAR(iter->second.buildingName));
			strStair.Format(_T("%s"), CCommonFunc::CharToWCHAR(iter->second.stair));
			strFloor.Format(_T("%s"), CCommonFunc::CharToWCHAR(iter->second.floor));
			strRoom.Format(_T("%s"), CCommonFunc::CharToWCHAR(iter->second.room));
			strCircuit.Format(_T("%s"), CCommonFunc::CharToWCHAR(iter->second.circuitName));

			Server::Instance()->SendAll((BYTE*)&gpue, sizeof(GFSProtocolUnsolicitedEvent));
			strMsg.Format(_T("이벤트 발생 ! - 건물 : [%s], 계단 : [%s], 층 : [%s], 실 : [%s], 설비 : [%s], EVENT TYPE : [%s], 발생 정보 : [%s]"),
				strBuildingName,
				strStair,
				strFloor,
				strRoom,
				strCircuit,
				strEventType,
				strOccurence
				);
			Log::Trace("%s", CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
		}
		else
		{
			Log::Trace("이벤트에 대한 회로 위치 정보를 찾을 수 없어서 이벤트 전송을 하지 않습니다.");
			return;
		}
	}
	else  //수신기가 복구되면 각 이벤트 타입 별로 일괄해제를 보낸다. -> 김호 마스터 -> 20230817 GBM - 수신기 복구 시 감시 이벤트 수천건 발생으로 인해 불필요한 알람 방지를 위해 수신기 복구 이벤트만 전송
	{
		//회로 위치 정보가 없으므로 헤더와 eventType, occurrence, reserved를 제외한 나머지 정보를 0으로 초기화 
		memset(gpue.building, 0, sizeof(GFSProtocolUnsolicitedEvent) - (sizeof(GFSProtocolHeader) + sizeof(int) /* eventType */ + sizeof(int) /* occurrence */  + sizeof(double) /* reserved */));

		for (int i = GFSProtocolHeader::GFSProtocolEventType::FIRE_INFO; i <= GFSProtocolHeader::GFSProtocolEventType::RESTORATION_INFO; i++)
		{
			gpue.eventType = i;
			switch (i)
			{
			case GFSProtocolHeader::GFSProtocolEventType::FIRE_INFO:
			{
				strEventType = _T("화재");
				//break;
				continue;
			}
			case GFSProtocolHeader::GFSProtocolEventType::GAS_INFO:
			{
				strEventType = _T("가스");
				//break;
				continue;
			}
			case GFSProtocolHeader::GFSProtocolEventType::SURVEILLANCE_INFO:
			{
				strEventType = _T("감시");
				//break;
				continue;
			}
			case GFSProtocolHeader::GFSProtocolEventType::DISCONNECTION_INFO:
			{
				strEventType = _T("단선");
				//break;
				continue;
			}
			case GFSProtocolHeader::GFSProtocolEventType::RESTORATION_INFO:
			{
				strEventType = _T("시스템 복구 신호");
				break;
			}
			default:
			{
				strEventType = _T("알 수 없는 이벤트 타입");
				break;
			}
			}

			Server::Instance()->SendAll((BYTE*)&gpue, sizeof(GFSProtocolUnsolicitedEvent));
			strMsg.Format(_T("이벤트 발생 ! - EVENT TYPE : [%s], 발생 정보 : [일괄 복구]"), strEventType);
			Log::Trace("%s", CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
			
			Sleep(100);	//sleep을 주지 않으면 패킷이 뭉쳐서 가능 경우가 발생해 받는 쪽에서 쪼개서 파싱해야 해서 잠시 sleep을 줌, 이쪽에서 구현하는 거 아니니까 크게 상관없긴 함
		}
	}
}