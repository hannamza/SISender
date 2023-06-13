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

void Client::ProcessResponseGetUserTokenList(BYTE* pData)		// ����: ����� ��� ��û
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
		Log::Trace("�α��� ����");
	}
	else
	{
		Log::Trace("�α��� ����");
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
		Log::Trace("�̺�Ʈ ���� ����");
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
		//nResult ���� ���� ���� ����Ǿ����� Ȯ��, ���� �ð����� �ʹ� ������ ���� ������ Ȯ��
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
	case KOCOM_ERROR_AUTH_FAIL_ID:	//�����δ� PW�� �߸��Ǿ Kocom ������ �̰ɷ� �����Ƿ� �޼��� ����
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

	//��� ���� �޼��� ���� ������ ���α׷� ����
	Log::Trace("Kocom �����κ��� Error Message�� ������ ���α׷��� �����մϴ�.");
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
		Log::Trace("�����κ��� �߸��� Command�� ���ŵǾ����ϴ�. �ش� ��Ŷ�� ó������ �ʽ��ϴ�. (command : %d)", command);	// ��� �� ���� ������ ��Ŷ�� ���� �ʴ� �̻� ����� ������ ����
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
		Log::Trace("�α��� ����!");
		break;
	}
	case GFSProtocolHeader::GFSProtocolCommand::KEEP_ALIVE_RESPONSE:
	{
// 		Log::Trace("Keep Alive �������! : %d");
		CCommonState::Instance()->m_dwLastRecv = GetTickCount();
		break;
	}
	case GFSProtocolHeader::GFSProtocolCommand::UNSOLICITED_EVENT:
	{
		//Log::Trace("ȭ�� �̺�Ʈ ����!");
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
		Log::Trace("�������� ���� ���� ������ ���ŵǾ����ϴ�.");
		break;
	}
	case GFSProtocolHeader::GFSProtocolResult::WRONG_LOGIN_ID:
	{
		Log::Trace("Login ID�� �߸��Ǿ����ϴ�.");
		break;
	}
	case GFSProtocolHeader::GFSProtocolResult::WRONG_LOGIN_PW:
	{
		Log::Trace("Login PW�� �߸��Ǿ����ϴ�.");
		break;
	}
	case GFSProtocolHeader::GFSProtocolResult::UNKNOWN_COMMAND:
	{
		Log::Trace("��û�� command�� �߸��Ǿ����ϴ�.");		// ��� �� ���� ������ ��Ŷ�� ���� �ʴ� �̻� ����� ������ ����
		break;
	}
	case GFSProtocolHeader::GFSProtocolResult::WRONG_PACKET_SIZE:
	{
		Log::Trace("Request Packet�� ũ�Ⱑ �߸��Ǿ����ϴ�.");
		break;
	}
	default:
	{
		Log::Trace("�������� ���� Result ���� ���Խ��ϴ�.");
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
		strEventType = _T("ȭ��");
		break;
	}
	case GFSProtocolHeader::GFSProtocolEventType::GAS_INFO:
	{
		strEventType = _T("����");
		break;
	}
	case GFSProtocolHeader::GFSProtocolEventType::SURVEILLANCE_INFO:
	{
		strEventType = _T("����");
		break;
	}
	case GFSProtocolHeader::GFSProtocolEventType::DISCONNECTION_INFO:
	{
		strEventType = _T("�ܼ�");
		break;
	}
	case GFSProtocolHeader::GFSProtocolEventType::RESTORATION_INFO:
	{
		strEventType = _T("�ý��� ���� ��ȣ");
		break;
	}
	default:
	{
		Log::Trace("���ǵ��� ���� EVENT TYPE�� �޾ҽ��ϴ�.");
		return;
	}
	}

	switch (pGpue->occurrence)
	{
	case GFSProtocolHeader::GFSProtocolOccurrence::OCCURRENCE:
	{
		strOccurInfo = _T("�߻�");
		break;
	}
	case GFSProtocolHeader::GFSProtocolOccurrence::RESTORATION:
	{
		strOccurInfo = _T("����");
		break;
	}
	case GFSProtocolHeader::GFSProtocolOccurrence::RESTORATION_ALL:
	{
		strOccurInfo = _T("�ϰ� ����");
		break;
	}
	default:
	{
		Log::Trace("���ǵ��� ���� �߻� ������ �޾ҽ��ϴ�.");
		return;
	}
	}

	strEventType.Format(_T("�̺�Ʈ �߻� ! - �ǹ� : [%s], ��� : [%s], �� : [%s], �� : [%s], ���� : [%s], EVENT TYPE : [%s], �߻� ���� [%s]"),
		strBuilding, strStair, strFloor, strRoom, strCircuit, strEventType, strOccurInfo);

	Log::Trace("%s", CCommonFunc::WCharToChar(strEventType.GetBuffer(0)));
}