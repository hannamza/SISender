
// SISenderDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "SISender.h"
#include "SISenderDlg.h"
#include "afxdialogex.h"

#include "TestDlg.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

UINT ThreadSMCheck(LPVOID pParam)
{
	CSISenderDlg* pDlg = (CSISenderDlg*)pParam;

	SYSTEMTIME st = CSM::ReadEventTimeFromSharedMemory();

	while (pDlg->m_bSMCheck)
	{
		if (pDlg->CheckSMTimeChanged(st, CSM::ReadEventTimeFromSharedMemory()))
		{
			BYTE buf[SI_EVENT_BUF_SIZE];
			CSM::ReadEventBufFromSharedMemory(buf, sizeof(buf));
			CEventSend::Instance()->SendEvent(buf);
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


// CSISenderDlg ��ȭ ����



CSISenderDlg::CSISenderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SISENDER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bInit = false;
	m_bKilled = false;
	m_bAutoLogin = false;

	m_nAliveCount = 0;
}

void CSISenderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_richLog);
	DDX_Control(pDX, IDC_CHECK_SCROLL, m_checkScroll);
	DDX_Control(pDX, IDC_BUTTON_TEST, m_btnTest);
}

BEGIN_MESSAGE_MAP(CSISenderDlg, CDialogEx)
	ON_MESSAGE(LOG_MESSAGE, OnLogMessage)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_START, &CSISenderDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CSISenderDlg::OnBnClickedButtonStop)
	ON_MESSAGE(WM_TRAY_NOTIFICATION, OnTrayNotification)
	ON_COMMAND(32771, &CSISenderDlg::OnMenuStart)
	ON_COMMAND(32772, &CSISenderDlg::OnMenuStop)
	ON_COMMAND(32773, &CSISenderDlg::OnMenuPopup)
	ON_COMMAND(32774, &CSISenderDlg::OnMenuExit)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CSISenderDlg::OnBnClickedButtonTest)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CSISenderDlg �޽��� ó����

BOOL CSISenderDlg::OnInitDialog()
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

	CCommonState::New();

	//�ڽ��� ���° SI_SENDER������ ����
#ifndef TEST_MODE_FLAG
	CCommonState::Instance()->m_nProcessIndex = _ttoi(theApp.m_lpCmdLine);
#else
	CCommonState::Instance()->m_nProcessIndex = 2;
#endif

	Log::Setup();
	Log::SetMainPointer(this);

	Log::Trace("SISender started...");
	Log::Trace("�� ���α׷��� SISender%d �Դϴ�.", CCommonState::Instance()->m_nProcessIndex);

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

	::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	m_bStarted = false;

	m_checkScroll.SetCheck(true);

#ifndef EVENT_TEST_MODE
	m_btnTest.ShowWindow(FALSE);
#endif

// 	TCHAR szPath[2048];
// 	GetModuleFileName(NULL, szPath, 2048);
// 	PathRemoveFileSpec(szPath);
// 	CString strINI_Name;
// 	strINI_Name.Format(_T("%s\\Setup.ini"), szPath);
// 
// 	TCHAR szTemp[32];
// 	memset(szTemp, 0x00, sizeof(szTemp));
// 	GetPrivateProfileString(L"COMM", L"PORT", L"COM1", szTemp, BUFF_SIZE, strINI_Name);
// 
// 	CString sText;
// 	bool bFind = false;
// 	for (int nIndex = 1; nIndex < 20; nIndex++)
// 	{
// 		sText.Format(L"COM%d", nIndex);
// 		m_comboPort.InsertString(m_comboPort.GetCount(), sText);
// 		if (sText.CompareNoCase(szTemp) == 0)
// 		{
// 			bFind = true;
// 			m_comboPort.SetCurSel(nIndex - 1);
// 		}
// 	}
// 	if (!bFind)
// 	{
// 		m_comboPort.SetCurSel(0);
// 	}

	//INI���� �о ��ü ������ �µ��� ��� ����
	BOOL bReadIni = FALSE;
	bReadIni = ReadIniFile();
	if (!bReadIni)
	{
		AfxMessageBox(_T("INI ���� �б⿡ �����߽��ϴ�! ���α׷��� �����մϴ�."));
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	//� SI��ü���� ǥ��
	switch (CCommonState::Instance()->m_nSIType)
	{
	case SI_TEST:
	{
		Log::Trace("�� ���α׷��� [TEST]�� ����մϴ�.");
		break;
	}
	case KOCOM:
	{
		Log::Trace("�� ���α׷��� [KOCOM]�� ����մϴ�.");
		break;
	}
	default:
	{
		break;
	}
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
	
	CClientInterface::New();
	CClientInterface::Instance()->TryConnection(CCommonState::Instance()->m_szServerIP, CCommonState::Instance()->m_nPort);	// 20230111 GBM - INI�� ����� �ܺξ�ü PORT�� ���� ��

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);

	//INI ���������� ������ ����
	SetTimer(3, 10, NULL);

	int nKeepAlivePeriod = 0;
	nKeepAlivePeriod = GetKeepAlivePeriod();

	if(nKeepAlivePeriod != 0)
		SetTimer(99, nKeepAlivePeriod, NULL);

	if (CCommonState::Instance()->m_nIdx == 0)
	{
		m_bAutoLogin = true;
		ShowWindow(SW_SHOW);
	}
	else
	{
		SetTimer(20, 100, NULL);
	}

	SetTimer(TIMER_ALIVE_COUNT_ID, TIMER_ALIVE_COUNT_PERIOD, NULL);

#ifndef TEST_MODE_FLAG
	SetTimer(TIMER_PM_QUIT_CHECK_ID, TIMER_PM_QUIT_CHECK_PERIOD, NULL);	//20230118 GBM - test
#endif

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CSISenderDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CSISenderDlg::OnPaint()
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
HCURSOR CSISenderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSISenderDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.

	if (nIDEvent == 2)
	{
		KillTimer(2);
		Log::Trace("���� ����(Serial ���� ����) - ���� �Ϸ�");
		OnBnClickedButtonStop();

		/*CEventSend::New();
		CEventSend::Instance()->SetMainWnd(this);
		CEventSend::Instance()->InitEvent();

		OnBnClickedButtonSend();*/
	}
	else if (nIDEvent == 3)
	{
		KillTimer(3);
		RegistryTrayIcon();
		GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(false);
		OnBnClickedButtonStart();
	}
	else if (nIDEvent == 10) {
		GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(false);
		OnBnClickedButtonStart();
	}
	else if (nIDEvent == 20) {
		KillTimer(20);
		m_bAutoLogin = true;
	}
	else if (nIDEvent == 99)
	{
		switch (CCommonState::Instance()->m_nSIType)
		{
		case SI_TEST:
		{
			if (GetTickCount() - CCommonState::Instance()->m_dwLastRecv >= 2500) {
				CClientInterface::Instance()->ProcessCommonRequest(ProtocolHeader::Keep_Alive, 0);
			}
			if (GetTickCount() - CCommonState::Instance()->m_dwLastRecv > 7 * 1000) {
				KillTimer(99);

				m_bKilled = true;
				OnMenuExit();
				return;
				/*CClientInterface::Delete();
				CClientInterface::New();
				CClientInterface::Instance()->TryConnection(m_szServerIP, 10234);
				g_nTest = 0;
				SetTimer(99, 1000, NULL);
				return;
				}
				++g_nTest;*/
			}

			break;
		}
		case KOCOM:
		{
			//CCommonState::Instance()->m_dwLastRecv = GetTickCount();
			CClientInterface::Instance()->KOCOMProcessRequestAlive();

			DWORD dw = GetTickCount();
			if (dw - CCommonState::Instance()->m_dwLastRecv > 60 * 1000)	// �ֱⰡ 50���̹Ƿ� ���� ������ keep alive ack �ð��� 10�� ������ ���� �� �̻��̸� �����̻����� �Ǵ�
			{
				//Keep Alive Ack�� �ʰ� �����߰ų� ���ʿ� ����� �ο����� ���� ����̹Ƿ� ���α׷� ����
				Log::Trace("Kocom Keep Alive Ack Too Late!");
				AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
			}
			else
			{
				//Log::Trace("Kocom Keep Alive Ack Received!");
			}
			break;
		}
		default:
			break;
		}

	}
	else if (nIDEvent == TIMER_ALIVE_COUNT_ID)
	{
		if (m_nAliveCount != ULLONG_MAX)
			m_nAliveCount++;
		else
			m_nAliveCount = 0;

		CSM::WriteAliveCountToSharedMemory(CCommonState::Instance()->m_nProcessIndex, m_nAliveCount);
	}
	else if (nIDEvent == TIMER_PM_QUIT_CHECK_ID)
	{
		bool bProcessRun = false;
		bProcessRun = CSM::ReadProcessRunFromSharedMemory(CCommonState::Instance()->m_nProcessIndex);

		if (!bProcessRun)
		{
			KillTimer(TIMER_PM_QUIT_CHECK_ID);
			Log::Trace("PM�� ���� ����!");
			PostMessageW(WM_QUIT);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CSISenderDlg::OnBnClickedButtonStart()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_bStarted)
	{
		return;
	}
	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(false);

	Log::Trace("Sender start...");

	// 20230111 GBM - FiCon���� �޾ƿ� �ǹ�-ȸ�� ���� ������ ����Ǿ�� ��
	CDeviceInfo::New();
	CDeviceInfo::Instance()->SetDeviceInfo();

	G_hShutdown_Event = CreateEvent(NULL, TRUE, FALSE, NULL);

	CEventSend::New();
	CEventSend::Instance()->SetMainWnd(this);
	CEventSend::Instance()->InitEvent();		// 20230111 GBM - �����޸𸮿��� ����� ť�� ó���ϴ� ��ƾ

	m_bSMCheck = true;

// 	TCHAR szPath[2048];
// 	GetModuleFileName(NULL, szPath, 2048);
// 	PathRemoveFileSpec(szPath);
// 	CString strINI_Name;
// 	strINI_Name.Format(_T("%s\\Setup.ini"), szPath);
// 
// 	CString strTemp;
// 	m_comboPort.GetLBText(m_comboPort.GetCurSel(), strTemp);
// 	WritePrivateProfileString(L"COMM", L"PORT", strTemp, strINI_Name);
// 
// 	Log::Trace("COM PORT : [COM %d], 9600, 8, 0, 1, 0", m_comboPort.GetCurSel() + 1);
// 
// 	CSerialComm::New();
// 	CSerialComm::Instance()->SetMainWnd(this);
// 	CSerialComm::Instance()->SetSerialCommPort(strTemp);
// 	//CSerialComm::Instance()->SetSerialBaudRate(GetPrivateProfileInt(L"COMM", L"BAUDRATE", 9600, strINI_Name));
// 	CSerialComm::Instance()->SetSerialBaudRate(GetPrivateProfileInt(L"COMM", L"BAUDRATE", 38400, strINI_Name));
// 	CSerialComm::Instance()->SetSerialDataBit(GetPrivateProfileInt(L"COMM", L"DATABIT", 8, strINI_Name));
// 	CSerialComm::Instance()->SetSerialParityBit(GetPrivateProfileInt(L"COMM", L"PARITY", 0, strINI_Name));
// 	CSerialComm::Instance()->SetSerialStopBit(GetPrivateProfileInt(L"COMM", L"STOPBIT", 1, strINI_Name));
// 	CSerialComm::Instance()->SetFACPNum(GetPrivateProfileInt(L"COMM", L"FACP_NUM", 0, strINI_Name));
// 	if (!CSerialComm::Instance()->Init())
// 	{
// 		GetDlgItem(IDC_BUTTON_START)->EnableWindow(false);
// 
// 		SetTimer(2, 3000, NULL);
// 		m_bStarted = true;
// 
// 		Log::Trace("���� ����(Serial ���� ����) - ���� ��.....");
// 
// 		return;
// 	}

	CWinThread* pThread = AfxBeginThread((AFX_THREADPROC)ThreadSMCheck, this);

	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(false);
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(false);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(true);

	m_bStarted = true;
}


void CSISenderDlg::OnBnClickedButtonStop()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (!m_bStarted)
	{
		return;
	}
	Log::Trace("Sender stop...");

	m_bSMCheck = false;

	Sleep(10);

	CEventSend::Delete();
	CDeviceInfo::Delete();

// 	::SetEvent(G_hShutdown_Event);
// 	WaitForSingleObject(CSerialComm::Instance()->m_hThreadWatchComm, INFINITE);
// 
// 	CSerialComm::Instance()->Close();
// 	CSerialComm::Delete();

	m_bStarted = false;

	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);
}

void CSISenderDlg::RegistryTrayIcon()
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

DWORD g_dwErrorTime = 0;
LRESULT CSISenderDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
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

void CSISenderDlg::OnMenuStart()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	OnBnClickedButtonStart();
}


void CSISenderDlg::OnMenuStop()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	OnBnClickedButtonStop();
}


void CSISenderDlg::OnMenuPopup()
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


void CSISenderDlg::OnMenuExit()
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


BOOL CSISenderDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CSISenderDlg::OnBnClickedButtonTest()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.

	//
	CDeviceInfo::New();
	CDeviceInfo::Instance()->SetDeviceInfo();

	G_hShutdown_Event = CreateEvent(NULL, TRUE, FALSE, NULL);

	CEventSend::New();
	CEventSend::Instance()->SetMainWnd(this);
	CEventSend::Instance()->InitEvent();
	//

	CTestDlg dlg;
	dlg.DoModal();

	BYTE pData[20];
	memset(pData, NULL, 20);
	pData[0] = 'G';
	pData[1] = 'X';
	pData[2] = 'R';
	memcpy(&pData[3], "00", 2);
	memcpy(&pData[5], "00", 2);
	pData[7] = '-';
	pData[8] = '0';
	memcpy(&pData[9], "000", 3);
	memcpy(&pData[12], "\r\n", 2);

	CEventSend::Instance()->SendEvent(pData);

	Sleep(1000);

	//
	CEventSend::Delete();
	CDeviceInfo::Delete();

	::SetEvent(G_hShutdown_Event);
	//
}


void CSISenderDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	if (!m_bKilled) {
		CCommonFunc::KillProcess(L"Sender ���α׷� ����.exe");
	}

	m_bKilled = true;

	OnBnClickedButtonStop();

	CClientInterface::Instance()->TryShutdown();
	CClientInterface::Instance()->Destroy();
	CClientInterface::Delete();

	CCommonState::Delete();

	CCircuitLocInfo::Delete();

	Log::Cleanup();

	//
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //Ʈ���̾����� ����
}

#define		LIMIT_RICH_LOG		(1024*1024*2)
LRESULT CSISenderDlg::OnLogMessage(WPARAM wParam, LPARAM lParam)
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

BOOL CSISenderDlg::ReadIniFile()
{
	INT nTemp = 0;
	CReadWriteState state;
	CString sTemp;
	state.SetFileName(L"Setup.ini");

	//�� ��° ���μ������� Ȯ��
	CString strSection = _T("");
	strSection.Format(_T("SI_INFO_%d"), CCommonState::Instance()->m_nProcessIndex);

	if (state.ReadState(strSection, L"IP", sTemp))
	{
		strcpy_s(CCommonState::Instance()->m_szServerIP, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
	}
	else
	{
		Log::Trace("Section : %s Key : IP ������ ��� ���� �����߽��ϴ�.", strSection);
		return FALSE;
	}

	if (state.ReadState(strSection, L"PORT", nTemp))
	{
		CCommonState::Instance()->m_nPort = nTemp;
	}
	else
	{
		Log::Trace("Section : %s Key : PORT ������ ��� ���� �����߽��ϴ�.", strSection);
		return FALSE;
	}

	if (state.ReadState(strSection, L"ID", sTemp))
	{
		strcpy_s(CCommonState::Instance()->m_szID, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
	}
	else
	{
		Log::Trace("Section : %s Key : ID ������ ��� ���� �����߽��ϴ�.", strSection);
		return FALSE;
	}

	//SI ��ü TYPE ���� �б�
	if (state.ReadState(strSection, L"TYPE", nTemp))
	{
		CCommonState::Instance()->m_nSIType = nTemp;

		switch (nTemp)
		{
		case SI_TEST:
		{

			if (state.ReadState(strSection, L"PW", sTemp))	//SI_TEST�� PW�� ��� �� ����
			{
				strcpy_s(CCommonState::Instance()->m_szPass, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
			}
			else
			{
				sTemp = _T("");
				strcpy_s(CCommonState::Instance()->m_szPass, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));

// 				Log::Trace("Section : %s Key : PW ������ ��� ���� �����߽��ϴ�.", strSection);
// 				return FALSE;
			}

			//SI_TEST�� ��� �߰� �κ�

			if (state.ReadState(L"INFO", L"LOGIN", nTemp) && nTemp == 1)	// 20230111 GBM - INI�� ����� �ܺξ�ü ���� ���� ID / PW�� ���� ��
			{
// 				state.ReadState(L"INFO", L"ID", sTemp);
// 				strcpy_s(CCommonState::Instance()->m_szID, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));

				state.ReadState(L"INFO", L"INDEX", nTemp);
				CCommonState::Instance()->m_nIdx = nTemp;

				CCommonState::Instance()->m_bLoginResult = true;

				ShowWindow(SW_HIDE);
				//SetTimer(3, 10, NULL);
			}
			else
			{
				Log::Trace("SI_TEST - Section : INFO Key : LOGIN ������ ��� ���� �����߽��ϴ�.", strSection);
				return FALSE;
			}

			break;
		}
		case KOCOM:
		{
			if (state.ReadState(strSection, L"PW", sTemp))
			{
				strcpy_s(CCommonState::Instance()->m_szPass, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
			}
			else
			{
				Log::Trace("Section : %s Key : PW ������ ��� ���� �����߽��ϴ�.", strSection);
				return FALSE;
			}
			break;
		}
		default:
		{
			break;
		}
		}
	}
	else
	{
		Log::Trace("Section : %s, Key : TYPE ������ ��� ���� �����߽��ϴ�.", strSection);
		return FALSE;
	}

	m_bInit = true;

	return TRUE;
}

int CSISenderDlg::GetKeepAlivePeriod()
{
	int nRet = 0;
	switch (CCommonState::Instance()->m_nSIType)
	{
	case SI_TEST:
	{
		nRet = 1000;
		break;
	}
	case KOCOM:
	{
		nRet = KOCOM_KEEP_ALIVE_PERIOD * 1000;	// 50sec * 1000ms
		break;
	}
	default:
		break;
	}

	return nRet;
}

void CSISenderDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	RegistryTrayIcon();
	return;

	CDialogEx::OnClose();
}

BOOL CSISenderDlg::CheckSMTimeChanged(SYSTEMTIME preTime, SYSTEMTIME curTime)
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
