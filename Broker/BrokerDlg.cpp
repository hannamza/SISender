
// BrokerDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "Broker.h"
#include "BrokerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CBrokerDlg ��ȭ ����



CBrokerDlg::CBrokerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_BROKER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bKilled = false;
	m_strLogFolderName = _T("Broker_Log");
	m_nAliveCount = 0;
}

void CBrokerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SCROLL, m_checkScroll);
	DDX_Control(pDX, IDC_COMBO1, m_comboPort);
	DDX_Control(pDX, IDC_RICHEDIT21, m_richLog);
}
BEGIN_MESSAGE_MAP(CBrokerDlg, CDialogEx)
	ON_MESSAGE(LOG_MESSAGE, OnLogMessage)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CBrokerDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CBrokerDlg::OnBnClickedButtonStop)
	ON_WM_TIMER()
	ON_MESSAGE(WM_TRAY_NOTIFICATION, OnTrayNotification)
	ON_COMMAND(32775, &CBrokerDlg::OnMenuStart)
	ON_COMMAND(32776, &CBrokerDlg::OnMenuStop)
	ON_COMMAND(32777, &CBrokerDlg::OnMenuPopup)
	ON_COMMAND(32778, &CBrokerDlg::OnMenuExit)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CBrokerDlg::OnBnClickedButtonTest)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CBrokerDlg �޽��� ó����

BOOL CBrokerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	Log::Setup();
	Log::SetMainPointer(this);

	m_checkScroll.SetCheck(true);

	BOOL bSMSucceed = FALSE;
	bSMSucceed = CSM::CreateSharedMemory();
	if (!bSMSucceed)
	{
		Log::Trace("�����޸� ������ �����߽��ϴ�. ���α׷��� �����մϴ�.");
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	Log::Trace("�����޸� ����!");

#ifndef EVENT_TEST_MODE
	GetDlgItem(IDC_BUTTON_TEST)->ShowWindow(FALSE);
#endif

	::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	m_bStarted = false;

	TCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);
	CString strINI_Name;
	strINI_Name.Format(_T("%s\\Setup.ini"), szPath);

	TCHAR szTemp[32];
	memset(szTemp, 0x00, sizeof(szTemp));
	GetPrivateProfileString(L"COMM", L"PORT", L"COM1", szTemp, BUFF_SIZE, strINI_Name);

	CString sText;
	bool bFind = false;
	for (int nIndex = 1; nIndex < 20; nIndex++)
	{
		sText.Format(L"COM%d", nIndex);
		m_comboPort.InsertString(m_comboPort.GetCount(), sText);
		if (sText.CompareNoCase(szTemp) == 0)
		{
			bFind = true;
			m_comboPort.SetCurSel(nIndex - 1);
		}
	}
	if (!bFind)
	{
		m_comboPort.SetCurSel(0);
	}

	SetTimer(3, 10, NULL);

	Log::Trace("Broker Start!");

	SetTimer(TIMER_ALIVE_COUNT_ID, TIMER_ALIVE_COUNT_PERIOD, NULL);

#ifndef TEST_MODE_FLAG
	SetTimer(TIMER_PM_QUIT_CHECK_ID, TIMER_PM_QUIT_CHECK_PERIOD, NULL);		//20230118 GBM -test
#endif

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CBrokerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CBrokerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CBrokerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CBrokerDlg::OnBnClickedButtonStart()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_bStarted)
	{
		return;
	}

	Log::Trace("���ű� �̺�Ʈ CHECK start...");

	G_hShutdown_Event = CreateEvent(NULL, TRUE, FALSE, NULL);

	TCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);
	CString strINI_Name;
	strINI_Name.Format(_T("%s\\Setup.ini"), szPath);

	CString strTemp;
	m_comboPort.GetLBText(m_comboPort.GetCurSel(), strTemp);
	WritePrivateProfileString(L"COMM", L"PORT", strTemp, strINI_Name);

	Log::Trace("COM PORT : [COM %d], 9600, 8, 0, 1, 0", m_comboPort.GetCurSel() + 1);

	CSerialComm::New();
	CSerialComm::Instance()->SetMainWnd(this);
	CSerialComm::Instance()->SetSerialCommPort(strTemp);
	CSerialComm::Instance()->SetSerialBaudRate(GetPrivateProfileInt(L"COMM", L"BAUDRATE", 38400, strINI_Name));
	CSerialComm::Instance()->SetSerialDataBit(GetPrivateProfileInt(L"COMM", L"DATABIT", 8, strINI_Name));
	CSerialComm::Instance()->SetSerialParityBit(GetPrivateProfileInt(L"COMM", L"PARITY", 0, strINI_Name));
	CSerialComm::Instance()->SetSerialStopBit(GetPrivateProfileInt(L"COMM", L"STOPBIT", 1, strINI_Name));
	CSerialComm::Instance()->SetFACPNum(GetPrivateProfileInt(L"COMM", L"FACP_NUM", 0, strINI_Name));
	if (!CSerialComm::Instance()->Init())
	{
		GetDlgItem(IDC_BUTTON_START)->EnableWindow(false);

		SetTimer(2, 3000, NULL);
		m_bStarted = true;

		Log::Trace("���� ����(Serial ���� ����) - ���� ��.....");

		return;
	}

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(false);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(true);

	m_bStarted = true;
}


void CBrokerDlg::OnBnClickedButtonStop()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	if (!m_bStarted)
	{
		return;
	}	
	Log::Trace("Broker stop...");

	::SetEvent(G_hShutdown_Event);
	WaitForSingleObject(CSerialComm::Instance()->m_hThreadWatchComm, INFINITE);

	CSerialComm::Instance()->Close();
	CSerialComm::Delete();

	m_bStarted = false;

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);
}


void CBrokerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.

	if (nIDEvent == 2)
	{
		KillTimer(2);
		Log::Trace("���� ����(Serial ���� ����) - ���� �Ϸ�");
		OnBnClickedButtonStop();	

	}
	else if (nIDEvent == 3)
	{
		KillTimer(3);
		RegistryTrayIcon();
		OnBnClickedButtonStart();
	}
	else if (nIDEvent == 10) 
	{
		OnBnClickedButtonStart();
	}
	else if (nIDEvent == TIMER_ALIVE_COUNT_ID)
	{
		if (m_nAliveCount != ULLONG_MAX)
			m_nAliveCount++;
		else
			m_nAliveCount = 0;

		CSM::WriteAliveCountToSharedMemory(BROKER, m_nAliveCount);
	}
	else if (nIDEvent == TIMER_PM_QUIT_CHECK_ID)
	{
		bool bProcessRun = false;
		bProcessRun = CSM::ReadProcessRunFromSharedMemory(BROKER);

		if (!bProcessRun)
		{
			KillTimer(TIMER_PM_QUIT_CHECK_ID);
			Log::Trace("PM�� ���� ����!");
			PostMessageW(WM_QUIT);
		}
			
	}

	CDialogEx::OnTimer(nIDEvent);
}

void CBrokerDlg::RegistryTrayIcon()
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	_tcscpy(nid.szTip, L"Broker");
	nid.uCallbackMessage = WM_TRAY_NOTIFICATION;

	BOOL bRet = ::Shell_NotifyIcon(NIM_ADD, &nid); //Ʈ���� ������ ���
	ShowWindow(SW_HIDE); //������ ���߱�
}

void CBrokerDlg::OnMenuStart()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	OnBnClickedButtonStart();
}


void CBrokerDlg::OnMenuStop()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	OnBnClickedButtonStop();
}


void CBrokerDlg::OnMenuPopup()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //Ʈ���̾����� ����
	if (!bRet)
	{
		Log::Trace("Ʈ���� ������ ���� ����");
		return;
	}
	ShowWindow(SW_SHOW); //������ Ȱ��ȭ
	ShowWindow(SW_RESTORE);
}


void CBrokerDlg::OnMenuExit()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //Ʈ���̾����� ����

	OnOK();
}

DWORD g_dwErrorTime = 0;
LRESULT CBrokerDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 999 && lParam == 999) { // ���� ����
		if (GetTickCount() - g_dwErrorTime >= 7500) {
			OnBnClickedButtonStop();
			SetTimer(10, 5000, NULL);
			g_dwErrorTime = GetTickCount();
		}
	}
	switch (lParam)
	{
	case WM_RBUTTONDOWN:
	{
		CPoint ptMouse;
		::GetCursorPos(&ptMouse);

		CMenu menu;
		menu.LoadMenu(IDR_MENU_TRAY);
		CMenu *pMenu = menu.GetSubMenu(0); //Ȱ��ȭ �� �޴� ����
		if (m_bStarted)
		{
			pMenu->EnableMenuItem(ID_START, MF_DISABLED);
			pMenu->EnableMenuItem(ID_STOP, MF_ENABLED);
		}
		else
		{
			pMenu->EnableMenuItem(ID_START, MF_ENABLED);
			pMenu->EnableMenuItem(ID_STOP, MF_DISABLED);
		}
		pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, ptMouse.x, ptMouse.y, AfxGetMainWnd());
	}
	break;

	case WM_LBUTTONDBLCLK:
	{
		NOTIFYICONDATA nid;
		ZeroMemory(&nid, sizeof(nid));
		nid.cbSize = sizeof(nid);
		nid.uID = 0;
		nid.hWnd = GetSafeHwnd();

		BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //Ʈ���̾����� ����
		if (!bRet)
		{
			Log::Trace("Ʈ���� ������ ���� ����");
			return -1;
		}
		ShowWindow(SW_SHOW); //������ Ȱ��ȭ
		ShowWindow(SW_RESTORE);
	}
	break;
	}
	return 0;
}

#define		LIMIT_RICH_LOG		(1024*1024*2)
LRESULT CBrokerDlg::OnLogMessage(WPARAM wParam, LPARAM lParam)
{
	CHAR * pLog = (CHAR*)wParam;
	if (strlen(pLog) > 512)
	{
		return 0;
	}
	TCHAR szLog[512];

	CTime time = CTime::GetCurrentTime();
	_stprintf_s(szLog, _T("[%d/%d %02d:%02d:%02d] %s"), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond(), CCommonFunc::CharToTCHAR(pLog));
	//_tcscpy_s(szLog, CCommonFunc::CharToTCHAR(pLog));

	OutputDebugString(szLog);

	long lLength = m_richLog.GetTextLengthEx(GTL_NUMCHARS);
	if (lLength > LIMIT_RICH_LOG)
	{
		m_richLog.SetSel(0, LIMIT_RICH_LOG / 10);
		m_richLog.ReplaceSel(L"");
	}

	CHARRANGE cr = { 0, 0 };
	m_richLog.GetSel(cr);
	m_richLog.SetSel(lLength, lLength);
	m_richLog.ReplaceSel(szLog);

	INT nMax = m_richLog.GetScrollLimit(SB_VERT);
	if (m_checkScroll.GetCheck())
	{
		m_richLog.SendMessage(WM_VSCROLL, MAKEWPARAM(SB_THUMBPOSITION, nMax), NULL);
		m_richLog.SendMessage(WM_VSCROLL, MAKEWPARAM(SB_ENDSCROLL, nMax), NULL);
	}

	return 0;
}

BOOL CBrokerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CBrokerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	if (!m_bKilled) {
		CCommonFunc::KillProcess(L"broker.exe");
	}

	OnBnClickedButtonStop();

	Log::Cleanup();

	//
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //Ʈ���̾����� ����
}


void CBrokerDlg::OnBnClickedButtonTest()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

// 	BYTE pData[15];
// 	memset(pData, NULL, 15);
// 	pData[0] = 'G';
// 	pData[1] = 'X';
// 	pData[2] = 'R';
// 	memcpy(&pData[3], "00", 2);
// 	memcpy(&pData[5], "00", 2);
// 	pData[7] = '-';
// 	pData[8] = '0';
// 	memcpy(&pData[9], "000", 3);
// 	memcpy(&pData[12], "\r\n", 2);
// 
// 	CSM::WriteEventToSharedMemory(pData);
// 	Log::Trace("Event Test - Shared Memory Wrote!");

	CDlgEventTest dlg;
	if (dlg.DoModal() == IDOK)
	{
		BYTE pData[SI_EVENT_BUF_SIZE];
		memset(pData, NULL, SI_EVENT_BUF_SIZE);
		memcpy(pData, dlg.m_eventBuf, SI_EVENT_BUF_SIZE);

		CSM::WriteEventToSharedMemory(pData);

		CString strBuf = _T("");
		for (int i = 0; i < SI_EVENT_BUF_SIZE; i++)
		{
			strBuf += pData[i];
		}
		Log::Trace("Event Test - [%s] Shared Memory is Written!", CCommonFunc::WCharToChar(strBuf.GetBuffer(0)));
	}
}


void CBrokerDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	
	//x��ư���� ���̾�α� ������ ���ᰡ �ƴ϶� â�� Ʈ���̷� ������ ����� �˾� �޴��θ� �����ϵ��� ��
	RegistryTrayIcon();
	return;

	CDialogEx::OnClose();
}
