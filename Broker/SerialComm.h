#include "stdafx.h"

#pragma once

class CSerialComm : public TSingleton<CSerialComm>
{
public:
	CSerialComm();
	~CSerialComm();

private:
	CString m_strCom_Port;
	int     m_nBaud;
	int     m_nBit;
	int     m_nParity;
	int     m_nStop_Bit;

	BYTE    m_RecvBuff[BUFF_SIZE];
	int     m_nRecvCnt;

	int     m_nWorking_FACP_Num;

	BYTE	*m_pRingBuffer;
	int		m_nRingBuffer;

public:
	BOOL         m_bCheck;

	CString		 m_strPortName;

	OVERLAPPED	 m_osRead;
	OVERLAPPED   m_osWrite;

	HANDLE		 m_hComm;
	HANDLE		 m_hThreadWatchComm;

	WORD		 m_wPortID;
	HWND		 m_hMessageWnd;
	CWnd*		 m_pWnd;

	BOOL		 m_bConnected;

	CHAR		 m_szLog[10][512];
	int			 m_nLogIndex;

public:
	BOOL	OpenPort(CString strPortName, DWORD dwBaud, BYTE byData, BYTE byStop, BYTE byParity);//포트 열기 
	void	ClosePort();
	DWORD	WriteComm(BYTE *pBuff, DWORD nToWrite);
	void	SetMainWnd(CWnd* pWnd);
	void    SetHWnd(HWND HWnd);
	HWND	GetHWnd();

	DWORD	ReadComm(BYTE *pBuff, DWORD nToRead);

	void    SetSerialCommPort(CString a_strPort_Name);
	void    SetSerialBaudRate(int a_nBaud_Rate);
	void    SetSerialDataBit(int a_nData_Bit);
	void    SetSerialParityBit(int a_nParity_Bit);
	void    SetSerialStopBit(int a_nStop_Bit);

	CString GetSerialCommPort();

	void    SetFACPNum(int a_nFACP_Num);

public:
	virtual bool Init();
	virtual void Close();
	virtual DWORD WriteData(BYTE *a_pBUFF, DWORD a_nData_Length);
};

DWORD	ThreadWatchComm(CSerialComm* pComm);

