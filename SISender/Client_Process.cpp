#include "stdafx.h"
#include "Client.h"
#include "ClientMan.h"

#include "Log.h"
#include "Network.h"

#include <cassert>
#include <iostream>
#include <vector>
#include <boost/pool/singleton_pool.hpp>
#include "SISenderDlg.h"

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
		KocomProcessResponseAlive(pData);
		break;
	case KOCOM_FIRE_TYPE | KOCOM_MSG_FIRE_ALARM_ACK:
		Log::Trace("Kocom Fire Alarm Ack Received!");
		break;
	case KOCOM_FIRE_TYPE | KOCOM_MSG_ERROR_ACK:
// 		int nResult;
// 		memcpy_s(&nResult, sizeof(int), &buffer[sizeof(KOCOMProtocolHeader)], sizeof(int));
// 		Log::Trace("Kocom Error Ack Received! - nResult : %d", nResult);
		KocomProcessResponseError(pData);
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
	case KOCOM_ERROR_AUTH_FAIL_ID:
	{
		Log::Trace("Kocom Error - ID is Wrong!");
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

	CSISenderDlg* pDlg = (CSISenderDlg*)AfxGetMainWnd();
	if (pDlg != NULL)
	{
		// 		pDlg->m_bKilled = true;
		// 		pDlg->OnMenuExit();
		pDlg->PostMessage(WM_CLOSE);
	}

}

void Client::KocomProcessResponseAlive(BYTE* pData)
{
	DWORD dw = GetTickCount();
// 	if (dw - CCommonState::Instance()->m_dwLastRecv > 10 * 1000)	// request 후 margin 10초 내
// 	{
// 		Log::Trace("Kocom Keep Alive Ack Too Late!");
// 		CSISenderDlg* pDlg = (CSISenderDlg*)AfxGetMainWnd();
// 		if (pDlg != NULL)
// 		{
// 			// 		pDlg->m_bKilled = true;
// 			// 		pDlg->OnMenuExit();
// 			pDlg->PostMessage(WM_CLOSE);
// 		}
// 	}
// 	else
// 	{
// 		//Log::Trace("Kocom Keep Alive Ack Received!");
// 	}

	CCommonState::Instance()->m_dwLastRecv = dw;
}