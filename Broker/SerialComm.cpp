#include "stdafx.h"
#include "SerialComm.h"
#include "EventSend.h"

extern CString G_strDebug;

CSerialComm::CSerialComm()
{
	m_bConnected = FALSE;
	m_nWorking_FACP_Num = 0;
	m_pWnd = NULL;
	m_nLogIndex = 0;
	m_nRecvCnt = 0;
	m_pRingBuffer = new BYTE[1024 * 1024];
	m_nRingBuffer = 0;
}

CSerialComm::~CSerialComm()
{
	SAFE_DELETE(m_pRingBuffer);
}

void CSerialComm::SetSerialCommPort(CString a_strPort_Name)
{
	m_strCom_Port = a_strPort_Name;
}

void CSerialComm::SetSerialBaudRate(int a_nBaud_Rate)
{
	m_nBaud = a_nBaud_Rate;
}

void CSerialComm::SetSerialDataBit(int a_nData_Bit)
{
	m_nBit = a_nData_Bit;
}

void CSerialComm::SetSerialParityBit(int a_nParity_Bit)
{
	m_nParity = a_nParity_Bit;
}

void CSerialComm::SetSerialStopBit(int a_nStop_Bit)
{
	m_nStop_Bit = a_nStop_Bit;
}

bool CSerialComm::Init()
{
	//CSimpleSenderDlg *pDlg = (CSimpleSenderDlg*)theApp.GetMainWnd();

	//SetHWnd(pDlg->GetSafeHwnd());

	if (m_bConnected)
	{
		ClosePort();

		Sleep(1000);
	}

	char szComport[16];
	strcpy_s(szComport, CCommonFunc::WCharToChar(m_strCom_Port.GetBuffer(0)));
	if (OpenPort(m_strCom_Port, m_nBaud, m_nBit, m_nStop_Bit - 1, m_nParity))
	{
		sprintf_s(m_szLog[m_nLogIndex], "Connection Success : %s Baud : %d Bit : %d Parity : %d StopBit : %d", szComport, m_nBaud, m_nBit, m_nParity, m_nStop_Bit);
		Log::Trace(m_szLog[m_nLogIndex]);
		//if(m_pWnd) m_pWnd->PostMessageW(LOG_MESSAGE, (WPARAM)m_szLog[m_nLogIndex], 0);
		if (++m_nLogIndex >= 10)
		{
			m_nLogIndex = 0;
		}
		//theApp.WriteLog(L"Connection Success : %s Baud : %d Bit : %d Parity : %d StopBit : %d", m_strCom_Port, m_nBaud, m_nBit, m_nParity, m_nStop_Bit);
		m_bConnected = true;

		return true;
	}
	else
	{
		sprintf_s(m_szLog[m_nLogIndex], "Connection Fail : %s Baud : %d Bit : %d Parity : %d StopBit : %d", szComport, m_nBaud, m_nBit, m_nParity, m_nStop_Bit);
		Log::Trace(m_szLog[m_nLogIndex]);
		//if(m_pWnd) m_pWnd->PostMessageW(LOG_MESSAGE, (WPARAM)m_szLog[m_nLogIndex], 0);
		if (++m_nLogIndex >= 10)
		{
			m_nLogIndex = 0;
		}
		//theApp.WriteLog(L"Connection Fail : %s Baud : %d Bit : %d Parity : %d StopBit : %d", m_strCom_Port, m_nBaud, m_nBit, m_nParity, m_nStop_Bit);

		DWORD dw = GetLastError();
	}
	return false;
}

void CSerialComm::Close()
{
	if (TRUE == m_bConnected)
	{
		ClosePort();
	}
}

DWORD CSerialComm::WriteData(BYTE *a_pBUFF, DWORD a_nData_Length)
{
	return WriteComm(a_pBUFF, a_nData_Length);
}

BOOL CSerialComm::OpenPort(CString strPortName, DWORD dwBaud, BYTE byData, BYTE byStop, BYTE byParity)
{
	COMMTIMEOUTS timeouts;
	DCB			 dcb;
	DWORD		 dwThreadID;

	//overlapped structure 변수 초기화.
	m_osRead.Offset = 0;
	m_osRead.OffsetHigh = 0;

	//Read 이벤트 생성에 실패..
	if (!(m_osRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))
	{
		return FALSE;
	}

	m_osWrite.Offset = 0;
	m_osWrite.OffsetHigh = 0;

	//Write 이벤트 생성에 실패..
	if (!(m_osWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))
	{
		return FALSE;
	}

	m_strPortName = strPortName;

	m_hComm = CreateFile(m_strPortName,
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
		NULL);

	if (m_hComm == (HANDLE)-1)
	{
		return FALSE;
	}

	//EV_RXCHAR event 설정...데이터가 들어오면.. 수신 이벤트가 발생하게끔..
	SetCommMask(m_hComm, EV_RXCHAR);

	//InQueue, OutQueue 크기 설정.
	SetupComm(m_hComm, BUFF_SIZE, BUFF_SIZE);

	// 포트 비우기.
	PurgeComm(m_hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);

	//timeout 설정.
	timeouts.ReadIntervalTimeout = 0xFFFFFFFF;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;

	timeouts.WriteTotalTimeoutMultiplier = 2 * CBR_9600 / dwBaud;
	timeouts.WriteTotalTimeoutConstant = 0;

	SetCommTimeouts(m_hComm, &timeouts);

	dcb.DCBlength = sizeof(DCB);

	GetCommState(m_hComm, &dcb);

	dcb.BaudRate = dwBaud;
	dcb.ByteSize = byData;
	dcb.Parity = byParity;
	dcb.StopBits = byStop;

	if (!SetCommState(m_hComm, &dcb))
	{
		return FALSE;
	}

	m_bConnected = TRUE;

	m_hThreadWatchComm = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadWatchComm, this, 0, &dwThreadID);

	if (!m_hThreadWatchComm)
	{
		ClosePort();
		return FALSE;
	}

	m_bCheck = FALSE;

	return TRUE;
}

void CSerialComm::ClosePort()
{
	m_bConnected = FALSE;

	SetCommMask(m_hComm, 0);

	PurgeComm(m_hComm, PURGE_TXABORT | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_RXCLEAR);

	WaitForSingleObject(m_hThreadWatchComm, INFINITE);

	CloseHandle(m_hComm);
}

void CSerialComm::SetFACPNum(int a_nFACP_Num)
{
	m_nWorking_FACP_Num = a_nFACP_Num;
}

DWORD CSerialComm::WriteComm(BYTE *pBuff, DWORD nToWrite)
{
	DWORD	dwWritten, dwError, dwErrorFlags;
	COMSTAT	comstat;

	if (!m_bConnected)
	{
		return 0;
	}

	TCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);
	CString strFileName;
	strFileName.Format(_T("%s\\debug.log"), szPath);


	if (L"Y" == G_strDebug) // && strlen((char *)pBuff) > 7)
	{
		//CString strFileName = theApp.GetAppPath() + L"\\debug.log";

		CString strLog;
		CString strChar;

		CTime time;
		time = CTime::GetCurrentTime();

		strLog.Format(L"%04d%02d%02d-%02d%02d%02d Write : ", time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());

		for (unsigned int i = 0; i < strlen((char *)pBuff); i++)
		{
			strChar.Format(L"%02X ", pBuff[i]);
			strLog = strLog + strChar;
		}

		strLog = strLog + L"\n";

		CFile LogFile;
		CFileException ex;

		if (LogFile.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite, &ex))
		{
			LogFile.SeekToEnd();

			int nLen = WideCharToMultiByte(CP_ACP, 0, strLog, -1, NULL, 0, NULL, NULL);

			char *szMsg = new char[nLen + 1];

			memset(szMsg, 0x00, nLen + 1);
			WideCharToMultiByte(CP_ACP, 0, strLog, -1, szMsg, nLen, 0, 0);

			LogFile.Write(szMsg, nLen - 1); // Delete Null Terminator

			delete[]szMsg;

			LogFile.Close();
		}
	}

	if (!WriteFile(m_hComm, pBuff, nToWrite, &dwWritten, &m_osWrite))
	{
		if (GetLastError() == ERROR_IO_PENDING)
		{
			while (!GetOverlappedResult(m_hComm, &m_osWrite, &dwWritten, TRUE))
			{
				dwError = GetLastError();
				if (dwError != ERROR_IO_INCOMPLETE)
				{
					ClearCommError(m_hComm, &dwErrorFlags, &comstat);
					break;
				}
			}
		}
		else
		{
			dwWritten = 0;
			ClearCommError(m_hComm, &dwErrorFlags, &comstat);
		}
	}

	return dwWritten;
}

DWORD CSerialComm::ReadComm(BYTE *pBuff, DWORD nToRead)
{
	DWORD	dwRead, dwError, dwErrorFlags;
	COMSTAT comstat;

	BYTE chData = 0;
	BYTE *pData;

	ClearCommError(m_hComm, &dwErrorFlags, &comstat);

	dwRead = comstat.cbInQue;
	if (dwRead > 0)
	{
		if (!ReadFile(m_hComm, pBuff, nToRead, &dwRead, &m_osRead))
		{
			if (GetLastError() == ERROR_IO_PENDING)
			{
				while (!GetOverlappedResult(m_hComm, &m_osRead, &dwRead, TRUE))
				{
					dwError = GetLastError();
					if (dwError != ERROR_IO_INCOMPLETE)
					{
						ClearCommError(m_hComm, &dwErrorFlags, &comstat);
						break;
					}
				}
			}
			else
			{
				dwRead = 0;
				ClearCommError(m_hComm, &dwErrorFlags, &comstat);
			}
		}
		TCHAR szPath[2048];
		GetModuleFileName(NULL, szPath, 2048);
		PathRemoveFileSpec(szPath);
		CString strFileName;
		strFileName.Format(_T("%s\\debug.log"), szPath);

		if (L"Y" == G_strDebug) 
		{
			//CString strFileName = theApp.GetAppPath() + L"\\debug.log";

			CString strLog;
			CString strChar;

			CTime time;
			time = CTime::GetCurrentTime();

			strLog.Format(L"%04d%02d%02d-%02d%02d%02d Read : ", time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());

			strChar.Format(L"(%d) ", dwRead);
			strLog = strLog + strChar;

			for (unsigned long i = 0; i < dwRead; i++)
			{
				strChar.Format(L"%02X ", pBuff[i]);
				strLog = strLog + strChar;
			}

			strLog = strLog + L"\n";
			CFile LogFile;
			CFileException ex;

			if (LogFile.Open(strFileName, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite, &ex))
			{
				LogFile.SeekToEnd();

				int nLen = WideCharToMultiByte(CP_ACP, 0, strLog, -1, NULL, 0, NULL, NULL);

				char *szMsg = new char[nLen + 1];

				memset(szMsg, 0x00, nLen + 1);
				WideCharToMultiByte(CP_ACP, 0, strLog, -1, szMsg, nLen, 0, 0);

				LogFile.Write(szMsg, nLen - 1); // Delete Null Terminator

				delete[]szMsg;

				LogFile.Close();
			}
		}

		for (int i = 0; i < (int)dwRead; i++)
		{
			chData = pBuff[i];

			if (NEW_INPUT == chData)
			{
				m_RecvBuff[m_nRecvCnt++] = chData;
				m_RecvBuff[m_nRecvCnt++] = 0x00;

				pData = &m_pRingBuffer[m_nRingBuffer];
				m_nRingBuffer += m_nRecvCnt + 1;
				if (m_nRingBuffer + 512 >= 1024 * 1024) {
					m_nRingBuffer = 0;
				}

				//pData = new BYTE[m_nRecvCnt+1];
				memset(pData, 0x00, m_nRecvCnt + 1);
				memcpy(pData, m_RecvBuff, m_nRecvCnt);

				//20230111 GBM start - 이벤트 공유메모리에 쓰도록 변경
				//CEventSend::Instance()->SendEvent(pData);
				CSM::WriteEventToSharedMemory(pData);
				CString strBuf = _T("");
				for (int i = 0; i < SI_EVENT_BUF_SIZE; i++)
				{
					strBuf += pData[i];
				}
				Log::Trace("Serial Event Occured! Shared Memory is Written!", CCommonFunc::WCharToChar(strBuf.GetBuffer(0)));
				//Log::Trace("Serial Event Occured! Shared Memory is Written!");
				//20230111 GBM end

				//theApp.ProcessData(pData);
				//Log::Trace("Send event: %s", (char*)pData);

				memset(m_RecvBuff, 0x00, BUFF_SIZE);
				m_nRecvCnt = 0;
			}
			else
			{
				m_RecvBuff[m_nRecvCnt] = chData;
				m_nRecvCnt++;

				if (BUFF_SIZE < m_nRecvCnt)
				{
					m_nRecvCnt = 0;
				}
			}
		}
	}

	return dwRead;
}

void CSerialComm::SetHWnd(HWND HWnd)
{
	m_hMessageWnd = HWnd;
}

HWND CSerialComm::GetHWnd()
{
	return m_hMessageWnd;
}

void CSerialComm::SetMainWnd(CWnd* pWnd)
{
	m_pWnd = pWnd;
}

CString CSerialComm::GetSerialCommPort()
{
	return m_strCom_Port;
}

DWORD ThreadWatchComm(CSerialComm* pComm)
{
	//CSimpleSenderDlg *pDlg = (CSimpleSenderDlg*)theApp.GetMainWnd();

	DWORD      dwEvent;
	OVERLAPPED os;
	BOOL       bOk = TRUE;
	BYTE       buff[BUFF_SIZE];
	DWORD      dwRead;
	DWORD      dwRtn;

	int        nCnt = 0;

	memset(&os, 0, sizeof(OVERLAPPED));

	if (!(os.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL)))
	{
		bOk = FALSE;
	}

	if (!SetCommMask(pComm->m_hComm, EV_RXCHAR))
	{
		bOk = FALSE;
	}

	if (!bOk)
	{
		return FALSE;
	}

	while (WAIT_OBJECT_0 != WaitForSingleObject(G_hShutdown_Event, 0))
	{
		dwEvent = 0;

		SetCommMask(pComm->m_hComm, EV_RXCHAR);

		if (!WaitCommEvent(pComm->m_hComm, &dwEvent, &os))
		{
			int dwError = GetLastError();
			if (ERROR_IO_PENDING != GetLastError())
			{
				char szText[64];
				sprintf_s(szText, "WaitCommEvent Error : %d", GetLastError());
				Log::Trace(szText);
				if (pComm->m_pWnd) {
					pComm->m_pWnd->PostMessage(WM_TRAY_NOTIFICATION, 999, 999);
				}
				//sprintf_s(pComm->m_szLog[pComm->m_nLogIndex], "WaitCommEvent Error : %d", GetLastError());
				//if(pComm->m_pWnd) pComm->m_pWnd->PostMessageW(LOG_MESSAGE, (WPARAM)pComm->m_szLog[pComm->m_nLogIndex], 0);
				/*if (++pComm->m_nLogIndex >= 10)
				{
					pComm->m_nLogIndex = 0;
				}*/
				//theApp.WriteLog(L"WaitCommEvent Error : %d", GetLastError());
			}
		}

		dwRtn = WaitForSingleObject(os.hEvent, 5000); //INFINITE); // 5000);

		if (!pComm->m_bConnected)
		{
			break;
		}

		if (WAIT_TIMEOUT == dwRtn)
		{
			os.Offset = 0;
			os.OffsetHigh = 0;
		}
		else if (WAIT_OBJECT_0 == dwRtn)
		{
			if ((dwEvent & EV_RXCHAR) == EV_RXCHAR)
			{
				memset(buff, 0x00, sizeof(buff));
				dwRead = pComm->ReadComm(buff, BUFF_SIZE);

				while (dwRead)
				{
					memset(buff, 0x00, sizeof(buff));
					dwRead = pComm->ReadComm(buff, BUFF_SIZE);
				}
			}
		}
	}

	CloseHandle(os.hEvent);

	return TRUE;
}	
