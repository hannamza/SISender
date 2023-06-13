#include "stdafx.h"
#include "Client.h"
#include "ClientMan.h"

#include "Log.h"
#include "Network.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <boost/pool/singleton_pool.hpp>
#include "GFSClientDlg.h"

using namespace std;

void Client::WorkerProcessRecvPacket(BYTE* pPacket, int nSize)
{
	BYTE* buffer = m_recvBuffer, *pData = NULL;

	USHORT command, size;
	memcpy_s(&command, sizeof(USHORT), buffer, sizeof(USHORT));
	memcpy_s(&size, sizeof(USHORT), &buffer[sizeof(USHORT)], sizeof(USHORT));
	if (size > 0)
	{
		pData = buffer;
	}
	if (command < ProtocolHeader::Keep_Alive || command >= ProtocolHeader::DefineEndProtocol)
	{
		return;
	}
	switch (command)
	{
	case ProtocolHeader::Keep_Alive:
		CCommonState::Instance()->m_dwLastRecv = GetTickCount();
		break;
	case ProtocolHeader::ResponseLogin:
		ProcessResponseLogin(pData);
		CCommonState::Instance()->m_dwLastRecv = GetTickCount();
		TRACE("ProtocolHeader::ResponseLogin\n");
		break;
	case ProtocolHeader::ResponseAddEvent:
		TRACE("ProtocolHeader::ResponseAddEvent\n");
		CCommonState::Instance()->m_dwLastRecv = GetTickCount();
		break;
	case ProtocolHeader::ResponseGetUserTokenList:
		ProcessResponseGetUserTokenList(pData);
		CCommonState::Instance()->m_dwLastRecv = GetTickCount();
		TRACE("ProtocolHeader::ResponseGetUserTokenList\n");
		break;
	default:
		break;
	}

	m_recvBuffer[nSize] = '\0';
}

void Client::ProcessResponseGetUserTokenList(BYTE* pData)		// 응답: 사용자 목록 요청
{
	ProtocolResponseGetUserTokenList* pList = (ProtocolResponseGetUserTokenList*)pData;
	int nCount = pList->nCount;

	CReadWriteState state;
	state.SetFileName(L"Setup.ini");
	if (!state.WriteState(L"ID", L"ID_CNT", nCount)) {
		return;
	}

	CString strField_Name;
	for (int nIndex = 0; nIndex < nCount; nIndex++)
	{
		/*strField_Name.Format(L"ID%d", nIndex + 1);
		state.WriteState(L"ID", strField_Name, CCommonFunc::Utf8ToWCHAR(pList->info[nIndex].szToken));
		state.WriteState(L"ID", strField_Name, CCommonFunc::Utf8ToWCHAR(pList->info[nIndex].szToken));*/
		strField_Name.Format(L"ID%d", nIndex + 1);
		state.WriteState(strField_Name, L"TOKEN", CCommonFunc::Utf8ToWCHAR(pList->info[nIndex].szToken));
		state.WriteState(strField_Name, L"TYPE", pList->info[nIndex].nMobileType);
		state.WriteState(strField_Name, L"FIRE", pList->info[nIndex].nFire);
		state.WriteState(strField_Name, L"GAS", pList->info[nIndex].nGas);
		state.WriteState(strField_Name, L"SPY", pList->info[nIndex].nSpy);
		state.WriteState(strField_Name, L"LINE", pList->info[nIndex].nLine);
		state.WriteState(strField_Name, L"ALERT", pList->info[nIndex].nAlert);
		state.WriteState(strField_Name, L"HOUR", pList->info[nIndex].nHour);
		state.WriteState(strField_Name, L"MIN", pList->info[nIndex].nMin);
		state.WriteState(strField_Name, L"ENDHOUR", pList->info[nIndex].nEndHour);
		state.WriteState(strField_Name, L"ENDMIN", pList->info[nIndex].nEndMin);
		state.WriteState(strField_Name, L"USETIME", pList->info[nIndex].nUseTime);
		state.WriteState(strField_Name, L"PHONE", CCommonFunc::Utf8ToWCHAR(pList->info[nIndex].szPhoneNo));
	}

	if (CEventSend::Instance()) {
		CEventSend::Instance()->InitEvent();
	}
	CCommonState::Instance()->m_dwToken = GetTickCount();
}

void Client::ProcessResponseLogin(BYTE* pData)
{
	ProtocolResponse* pReq = (ProtocolResponse*)pData;

	int idx = pReq->nResult;
	if (idx == 0)
	{
		Log::Trace("로그인 실패");
	}
	else
	{
		Log::Trace("로그인 성공");
		CCommonState::Instance()->m_nIdx = idx;
	}
	CCommonState::Instance()->m_bLoginResult = true;
}

void Client::ProcessResponseAddEvent(BYTE* pData)
{
	ProtocolResponse* pRes = (ProtocolResponse*)pData;

	int idx = pRes->nResult;
	if (idx == 0)
	{
		Log::Trace("이벤트 저장 실패");
	}
}

//Kocom
void Client::KocomWorkerProcessRecvPacket(BYTE* pPacket, int nSize)
{
	BYTE* buffer = m_recvBuffer, *pData = NULL;

	int headerkey, msgType, msgLength;

	memcpy_s(&headerkey, sizeof(int), buffer, sizeof(int));
	if (headerkey != KOCOM_HEADER_KEY_VALUE)
		return;

	memcpy_s(&msgType, sizeof(int), &buffer[sizeof(int)], sizeof(int));

	switch (msgType)
	{
	case KOCOM_FIRE_TYPE | KOCOM_MSG_BIND_ACK:
	{
		Log::Trace("Kocom Bind Succeed!");
		break;
	}
	case KOCOM_FIRE_TYPE | KOCOM_MSG_ALIVE_ACK:
		//nResult 값이 기존 값과 변경되었는지 확인, 이후 시간값이 너무 벌어져 있지 않은지 확인
		KocomProcessResponseAlive(buffer);
		break;
	case KOCOM_FIRE_TYPE | KOCOM_MSG_FIRE_ALARM_ACK:
		Log::Trace("Kocom Fire Alarm Ack Received!");
		break;
	case KOCOM_FIRE_TYPE | KOCOM_MSG_ERROR_ACK:
// 		int nResult;
// 		memcpy_s(&nResult, sizeof(int), &buffer[sizeof(KOCOMProtocolHeader)], sizeof(int));
// 		Log::Trace("Kocom Error Ack Received! - nResult : %d", nResult);
		KocomProcessResponseError(buffer);
	default:
		break;
	}

	m_recvBuffer[nSize] = '\0';

}

void Client::KocomProcessResponseError(BYTE* pData)
{
	KOCOMProtocolErrorAck* pErrorAck = (KOCOMProtocolErrorAck*)pData;

	int nResult = 0;
	nResult = pErrorAck->nResult;

	switch (nResult)
	{
	case KOCOM_ERROR_AUTH_FAIL_ID:	//실제로는 PW가 잘못되어도 Kocom 서버가 이걸로 보내므로 메세지 수정
	{
		//Log::Trace("Kocom Error - ID is Wrong!");
		Log::Trace("Kocom Error - ID or PW is Wrong!");
		break;
	}
	case KOCOM_ERROR_AUTH_FAIL_PASS:
	{
		Log::Trace("Kocom Error - PASSWORD is Wrong!");
		break;
	}
	case KOCOM_ERROR_OFF_HOME:
	{
		Log::Trace("Kocom Error - HOME OFF!");
		break;
	}
	case KOCOM_ERROR_RE_TRY_DATA:
	{
		Log::Trace("Kocom Error - RETRY DATA!");
		break;
	}
	case KOCOM_ERROR_OVERFLOW_Q_SIZE:
	{
		Log::Trace("Kocom Error - OVERFLOW!");
		break;
	}
	default:
	{
		Log::Trace("Kocom Error - Unknown Error!");
		break;
	}
	}

	//모든 에러 메세지 리턴 받으면 프로그램 종료
	Log::Trace("Kocom 서버로부터 Error Message를 수신해 프로그램을 종료합니다.");
	AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
}

void Client::KocomProcessResponseAlive(BYTE* pData)
{
	CCommonState::Instance()->m_dwLastRecv = GetTickCount();
}

//GFS

void Client::GFSWorkerProcessRecvPacket(BYTE* pPacket, int nSize)
{
	BYTE* buffer = m_recvBuffer, *pData = NULL;

	USHORT command, size;
	memcpy_s(&command, sizeof(USHORT), buffer, sizeof(USHORT));
	memcpy_s(&size, sizeof(USHORT), &buffer[sizeof(USHORT)], sizeof(USHORT));

	if (size > 0)
	{
		pData = buffer;
	}
	if ((command < GFSProtocolHeader::GFSProtocolCommand::ERROR_RESPONSE) || (command > GFSProtocolHeader::GFSProtocolCommand::UNSOLICITED_EVENT))
	{
		Log::Trace("서버로부터 잘못된 Command가 수신되었습니다. 해당 패킷을 처리하지 않습니다. (command : %d)", command);	// 통신 상 오류 비정상 패킷이 되지 않는 이상 여기로 들어오지 않음
		return;
	}

	switch (command)
	{
	case GFSProtocolHeader::GFSProtocolCommand::ERROR_RESPONSE:
	{
		GFSProcessError(pData);
		break;
	}
	case GFSProtocolHeader::GFSProtocolCommand::LOGIN_RESPONSE:
	{
		Log::Trace("로그인 성공!");
		break;
	}
	case GFSProtocolHeader::GFSProtocolCommand::KEEP_ALIVE_RESPONSE:
	{
// 		Log::Trace("Keep Alive 응답받음! : %d");
		CCommonState::Instance()->m_dwLastRecv = GetTickCount();
		break;
	}
	case GFSProtocolHeader::GFSProtocolCommand::UNSOLICITED_EVENT:
	{
		//Log::Trace("화재 이벤트 수신!");
		GFSProcessEvent(pPacket);
		break;
	}
	default:
		break;
	}
}

void Client::GFSProcessError(BYTE* pData)
{
	GFSProtocolError* pError = (GFSProtocolError*)pData;

	int nErrorCode = -1;
	nErrorCode = pError->result;

	switch (nErrorCode)
	{
	case GFSProtocolHeader::GFSProtocolResult::UNKNOWN_ERROR:
	{
		Log::Trace("지정되지 않은 오류 응답이 수신되었습니다.");
		break;
	}
	case GFSProtocolHeader::GFSProtocolResult::WRONG_LOGIN_ID:
	{
		Log::Trace("Login ID가 잘못되었습니다.");
		break;
	}
	case GFSProtocolHeader::GFSProtocolResult::WRONG_LOGIN_PW:
	{
		Log::Trace("Login PW가 잘못되었습니다.");
		break;
	}
	case GFSProtocolHeader::GFSProtocolResult::UNKNOWN_COMMAND:
	{
		Log::Trace("요청한 command가 잘못되었습니다.");		// 통신 상 오류 비정상 패킷이 되지 않는 이상 여기로 들어오지 않음
		break;
	}
	case GFSProtocolHeader::GFSProtocolResult::WRONG_PACKET_SIZE:
	{
		Log::Trace("Request Packet의 크기가 잘못되었습니다.");
		break;
	}
	default:
	{
		Log::Trace("지정되지 않은 Result 값이 들어왔습니다.");
		break;
	}
	}
}

void Client::GFSProcessEvent(BYTE* pData)
{
	GFSProtocolUnsolicitedEvent* pGpue = (GFSProtocolUnsolicitedEvent*)pData;
	
	CString strEventMsg = _T("");
	CString strBuilding = _T("");
	CString strStair = _T("");
	CString strFloor = _T("");
	CString strRoom = _T("");
	CString strCircuit = _T("");
	CString strEventType = _T("");
	CString strOccurInfo = _T("");

	WCHAR wcTest[30];
	memset(wcTest, 0, 30);
	strBuilding.Format(_T("%s"), CCommonFunc::Utf8ToWCHAR(pGpue->building));
	strStair.Format(_T("%s"), CCommonFunc::Utf8ToWCHAR(pGpue->stair));
	strFloor.Format(_T("%s"), CCommonFunc::Utf8ToWCHAR(pGpue->floor));
	strRoom.Format(_T("%s"), CCommonFunc::Utf8ToWCHAR(pGpue->room));
	strCircuit.Format(_T("%s"), CCommonFunc::Utf8ToWCHAR(pGpue->circuit));

	switch (pGpue->eventType)
	{
	case GFSProtocolHeader::GFSProtocolEventType::FIRE_INFO:
	{
		strEventType = _T("화재");
		break;
	}
	case GFSProtocolHeader::GFSProtocolEventType::GAS_INFO:
	{
		strEventType = _T("가스");
		break;
	}
	case GFSProtocolHeader::GFSProtocolEventType::SURVEILLANCE_INFO:
	{
		strEventType = _T("감시");
		break;
	}
	case GFSProtocolHeader::GFSProtocolEventType::DISCONNECTION_INFO:
	{
		strEventType = _T("단선");
		break;
	}
	case GFSProtocolHeader::GFSProtocolEventType::RESTORATION_INFO:
	{
		strEventType = _T("시스템 복구 신호");
		break;
	}
	default:
	{
		Log::Trace("정의되지 않은 EVENT TYPE을 받았습니다.");
		return;
	}
	}

	switch (pGpue->occurrence)
	{
	case GFSProtocolHeader::GFSProtocolOccurrence::OCCURRENCE:
	{
		strOccurInfo = _T("발생");
		break;
	}
	case GFSProtocolHeader::GFSProtocolOccurrence::RESTORATION:
	{
		strOccurInfo = _T("복구");
		break;
	}
	case GFSProtocolHeader::GFSProtocolOccurrence::RESTORATION_ALL:
	{
		strOccurInfo = _T("일괄 복구");
		break;
	}
	default:
	{
		Log::Trace("정의되지 않은 발생 정보를 받았습니다.");
		return;
	}
	}

	strEventType.Format(_T("이벤트 발생 ! - 건물 : [%s], 계단 : [%s], 층 : [%s], 실 : [%s], 설비 : [%s], EVENT TYPE : [%s], 발생 정보 [%s]"),
		strBuilding, strStair, strFloor, strRoom, strCircuit, strEventType, strOccurInfo);

	Log::Trace("%s", CCommonFunc::WCharToChar(strEventType.GetBuffer(0)));
}