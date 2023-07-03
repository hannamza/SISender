
// GFSServerDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "GFSServer.h"
#include "GFSServerDlg.h"
#include "afxdialogex.h"
#include "Server.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT ThreadSMCheck(LPVOID pParam)
{
	CGFSServerDlg* pDlg = (CGFSServerDlg*)pParam;

	SYSTEMTIME st = CSM::ReadEventTimeFromSharedMemory();

	while (pDlg->m_bSMCheck)
	{
		if (pDlg->CheckSMTimeChanged(st, CSM::ReadEventTimeFromSharedMemory()))
		{
			BYTE buf[SI_EVENT_BUF_SIZE];
			CSM::ReadEventBufFromSharedMemory(buf, sizeof(buf));
			CEventSend::Instance()->SendEvent(buf);
			Log::Trace("�����޸𸮿� �̺�Ʈ �߻���!");
			st = CSM::ReadEventTimeFromSharedMemory();

			//Log::Trace("�̺�Ʈ ó����!");
		}
	}

	return 0;
}

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


// CGFSServerDlg ��ȭ ����



CGFSServerDlg::CGFSServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GFSSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_nAliveCount = 0;
}

void CGFSServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_richLog);
	DDX_Control(pDX, IDC_CHECK_SCROLL, m_checkScroll);
}

BEGIN_MESSAGE_MAP(CGFSServerDlg, CDialogEx)
	ON_MESSAGE(LOG_MESSAGE, OnLogMessage)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_START, &CGFSServerDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CGFSServerDlg::OnBnClickedButtonStop)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TRAY_NOTIFICATION, OnTrayNotification)
	ON_COMMAND(ID_START, &CGFSServerDlg::OnMenuStart)
	ON_COMMAND(ID_STOP, &CGFSServerDlg::OnMenuStop)
	ON_COMMAND(ID_POPUP, &CGFSServerDlg::OnMenuPopup)
	ON_COMMAND(ID_EXIT, &CGFSServerDlg::OnMenuExit)
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CGFSServerDlg �޽��� ó����

BOOL CGFSServerDlg::OnInitDialog()
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

	//20230316 GBM start - github test
	int a = 0;
	//20230316 GBM end
	
	m_bStartServer = false;
	m_checkScroll.SetCheck(true);

	::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	Log::Setup();
	Log::SetMainPointer(this);

	Log::Trace("GFSServer started...");

	BOOL bSMSucceed = FALSE;
	bSMSucceed = CSM::CreateSharedMemory();
	if (!bSMSucceed)
	{
		Log::Trace("�����޸� ������ �����߽��ϴ�. ���α׷��� �����մϴ�.");
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	Log::Trace("�����޸� ����!");

	m_bSMCheck = true;

	CCommonData::New();

	BOOL bIniRead = FALSE;
	if (!GetServerInfoFromIniFile())
	{
		Log::Trace("INI ���� �б⿡ �����߽��ϴ�.");
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	CCircuitLocInfo::New();

	BOOL bSucceed = FALSE;
	bSucceed = CCircuitLocInfo::Instance()->GetCircuitLocInfoFromCSVFile();

	if (!bSucceed)
	{
		CStringArray strArr;
		strArr.RemoveAll();
		CCircuitLocInfo::Instance()->GetLocationTxtList(strArr);

		if (strArr.GetSize() == 0)
		{
			Log::Trace("ȸ�� ��ġ ���� ������ �д� ���� �����߽��ϴ�. ���α׷��� �����մϴ�.");
			AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
			return FALSE;
		}

		CCircuitLocInfo::Instance()->GetCircuitLocInfoFromTxtFile(strArr);

		if (CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.size() == 0)
		{
			Log::Trace("ȸ�� ��ġ ������ �д� ���� �����߽��ϴ�. ���α׷��� �����մϴ�.");
			AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
			return FALSE;
		}
	}

	CString sTemp = _T("");
	sTemp.Format(_T("%d"), CCommonData::Instance()->m_nServerPort);
	SetDlgItemText(IDC_EDIT_PORT, sTemp);

	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);

	SetTimer(1, 1000, NULL);

	SetTimer(2, 500, NULL);

	SetTimer(TIMER_ALIVE_COUNT_ID, TIMER_ALIVE_COUNT_PERIOD, NULL);

#ifndef TEST_MODE_FLAG
	SetTimer(TIMER_PM_QUIT_CHECK_ID, TIMER_PM_QUIT_CHECK_PERIOD, NULL);		//20230118 GBM -test
#endif

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CGFSServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CGFSServerDlg::OnPaint()
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
HCURSOR CGFSServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CGFSServerDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CGFSServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (nIDEvent == 1)
	{
		if (m_bStartServer)
		{
			int nAccept = Server::Instance()->GetNumClients();
			int nLimit = Server::Instance()->GetNumPostAccepts();

			CString sTemp;
			sTemp.Format(_T("%d"), nLimit);
			SetDlgItemText(IDC_EDIT_LIMIT, sTemp);
			sTemp.Format(_T("%d"), nAccept);
			SetDlgItemText(IDC_EDIT_CONNECT, sTemp);
		}
	}
	else if (nIDEvent == 2)
	{
		KillTimer(2);
		RegistryTrayIcon();
		OnBnClickedButtonStart();
	}
	else if (nIDEvent == TIMER_GFS_SERVER_KEEP_ALIVE_ID)
	{
		Server::Instance()->CheckKeepAliveGFSClients();
	}

	else if (nIDEvent == TIMER_ALIVE_COUNT_ID)
	{
		if (m_nAliveCount != ULLONG_MAX)
			m_nAliveCount++;
		else
			m_nAliveCount = 0;

		CSM::WriteAliveCountToSharedMemory(GFS_SERVER, m_nAliveCount);
	}
	else if (nIDEvent == TIMER_PM_QUIT_CHECK_ID)
	{
		bool bProcessRun = false;
		bProcessRun = CSM::ReadProcessRunFromSharedMemory(GFS_SERVER);

		if (!bProcessRun)
		{
			KillTimer(TIMER_PM_QUIT_CHECK_ID);
			Log::Trace("PM�� ���� ����!");
			PostMessageW(WM_QUIT);
		}

	}

	CDialogEx::OnTimer(nIDEvent);
}

#define		LIMIT_RICH_LOG		(1024*1024*2)
LRESULT CGFSServerDlg::OnLogMessage(WPARAM wParam, LPARAM lParam)
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


void CGFSServerDlg::OnBnClickedButtonStart()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	CString sPort;
	GetDlgItemText(IDC_EDIT_PORT, sPort);
	m_nServerPort = _ttoi(sPort.GetBuffer(0));
	if (m_nServerPort <= 0)
	{
		AfxMessageBox(_T("��Ʈ ��ȣ�� ���� 0���� ū ������ �Է� �����մϴ�."));
		return;
	}

	Log::Trace("Server starting....");

	m_bSMCheck = true;

	CCommonData::Instance()->m_nServerPort = m_nServerPort;
	sPort.ReleaseBuffer();
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (Network::Initialize() == false)
	{
		ERROR_MSG("Network::Initialize() failed");
		return;
	}

	Server::New();

	if (Server::Instance()->Create(CCommonData::Instance()->m_nServerPort, CCommonData::Instance()->m_nLimitClient) == false)
	{
		ERROR_MSG("Server::Create() failed");
		Network::Deinitialize();
		return;
	}

	Sleep(100);
	m_bStartServer = true;

	CEventSend::New();
	CEventSend::Instance()->SetMainWnd(this);
	CEventSend::Instance()->InitEvent();		// 20230111 GBM - �����޸𸮿��� ����� ť�� ó���ϴ� ��ƾ

	CWinThread* pThread = AfxBeginThread((AFX_THREADPROC)ThreadSMCheck, this);

	GetDlgItem(IDC_EDIT_PORT)->EnableWindow(false);

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(false);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(true);
	Log::Trace("Server started....");

	SetTimer(TIMER_GFS_SERVER_KEEP_ALIVE_ID, TIMER_GFS_SERVER_KEEP_ALIVE_PERIOD, NULL);
}


void CGFSServerDlg::OnBnClickedButtonStop()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	Log::Trace("Server stopping....");

	m_bSMCheck = false;

	KillTimer(TIMER_GFS_SERVER_KEEP_ALIVE_ID);

	m_bStartServer = false;
	Sleep(100);

	CEventSend::Delete();

	Server::Delete();

	Network::Deinitialize();

	Log::Trace("Server stopped....");

	GetDlgItem(IDC_EDIT_PORT)->EnableWindow(true);

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);

}

BOOL CGFSServerDlg::GetServerInfoFromIniFile()
{
	INT nTemp = 0;
	CReadWriteState state;
	CString sTemp = _T(""); 
	CString strSection = _T("");
	state.SetFileName(L"Setup.ini");

	strSection = _T("SI_INFO_SERVER");

	if (state.ReadState(strSection, L"PORT", nTemp))
	{
		CCommonData::Instance()->m_nServerPort = nTemp;
	}
	else
	{
		Log::Trace("GFS Server Port ������ INI ���Ͽ��� �о�� �� �����ϴ�.");
		return FALSE;
	}

	if (state.ReadState(strSection, L"ID", sTemp))
	{
		strcpy_s(CCommonData::Instance()->m_szID, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
	}
	else
	{
		Log::Trace("GFS Server ���� ID ������ INI ���Ͽ��� �о�� �� �����ϴ�.");
		return FALSE;
	}

	if (state.ReadState(strSection, L"PW", sTemp))
	{
		strcpy_s(CCommonData::Instance()->m_szPW, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
	}
	else
	{
		Log::Trace("GFS Server ���� PASSWORD ������ INI ���Ͽ��� �о�� �� �����ϴ�.");
		return FALSE;
	}

	return TRUE;
}

BOOL CGFSServerDlg::CheckSMTimeChanged(SYSTEMTIME preTime, SYSTEMTIME curTime)
{
	if (preTime.wMilliseconds != curTime.wMilliseconds)
		return TRUE;

	if (preTime.wSecond != curTime.wSecond)
		return TRUE;

	if (preTime.wMinute != curTime.wMinute)
		return TRUE;

	if (preTime.wHour != curTime.wHour)
		return TRUE;

	if (preTime.wDay != curTime.wDay)
		return TRUE;

	if (preTime.wMonth != curTime.wMonth)
		return TRUE;

	if (preTime.wYear != curTime.wYear)
		return TRUE;

	return FALSE;
}

void CGFSServerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	OnBnClickedButtonStop();

	KillTimer(2);

	CCommonData::Delete();

	CCircuitLocInfo::Delete();

	Log::Cleanup();
}

DWORD g_dwErrorTime = 0;
LRESULT CGFSServerDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
// 	if (wParam == 999 && lParam == 999) { // ���� ����
// 		if (GetTickCount() - g_dwErrorTime >= 7500) {
// 			OnBnClickedButtonStop();
// 			SetTimer(10, 5000, NULL);
// 			g_dwErrorTime = GetTickCount();
// 		}
// 	}
	switch (lParam)
	{
	case WM_RBUTTONDOWN:
	{
		CPoint ptMouse;
		::GetCursorPos(&ptMouse);

		CMenu menu;
		menu.LoadMenu(IDR_MENU_TRAY);
		CMenu *pMenu = menu.GetSubMenu(0); //Ȱ��ȭ �� �޴� ����
		if (m_bStartServer)
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

void CGFSServerDlg::OnMenuStart()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	OnBnClickedButtonStart();
}


void CGFSServerDlg::OnMenuStop()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	OnBnClickedButtonStop();
}


void CGFSServerDlg::OnMenuPopup()
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


void CGFSServerDlg::OnMenuExit()
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

void CGFSServerDlg::RegistryTrayIcon()
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	_tcscpy(nid.szTip, L"SISender");
	nid.uCallbackMessage = WM_TRAY_NOTIFICATION;

	BOOL bRet = ::Shell_NotifyIcon(NIM_ADD, &nid); //Ʈ���� ������ ���
	ShowWindow(SW_HIDE); //������ ���߱�
}

void CGFSServerDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	RegistryTrayIcon();
	return;

	CDialogEx::OnClose();
}
