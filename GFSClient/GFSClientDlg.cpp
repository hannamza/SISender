
// GFSClientDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "GFSClient.h"
#include "GFSClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
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


// CGFSClientDlg 대화 상자



CGFSClientDlg::CGFSClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_GFSCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bKilled = FALSE;
}

void CGFSClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_SCROLL, m_checkScroll);
	DDX_Control(pDX, IDC_RICHEDIT21, m_richLog);
}

BEGIN_MESSAGE_MAP(CGFSClientDlg, CDialogEx)
	ON_MESSAGE(LOG_MESSAGE, OnLogMessage)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_START, &CGFSClientDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CGFSClientDlg::OnBnClickedButtonStop)
	ON_WM_DESTROY()
END_MESSAGE_MAP()


// CGFSClientDlg 메시지 처리기

BOOL CGFSClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
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

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	CCommonState::New();

	Log::Setup();
	Log::SetMainPointer(this);

	Log::Trace("GFSClient started...");

	::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

	m_bStarted = false;

	m_checkScroll.SetCheck(true);

	//INI파일 읽어서 업체 정보에 맞도록 통신 설정
	BOOL bReadIni = FALSE;
	bReadIni = ReadIniFile();
	if (!bReadIni)
	{
		Log::Trace("INI 파일 읽기에 실패했습니다. 프로그램을 종료합니다.");
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	CClientInterface::New();

	CClientInterface::Instance()->TryConnection(CCommonState::Instance()->m_szServerIP, CCommonState::Instance()->m_nPort);

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);

	//INI 성공적으로 읽으면 실행
	SetTimer(3, 10, NULL);

	SetTimer(99, TIMER_GFS_SERVER_KEEP_ALIVE_PERIOD, NULL);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CGFSClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CGFSClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CGFSClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CGFSClientDlg::ReadIniFile()
{
	INT nTemp = 0;
	CReadWriteState state;
	CString sTemp;
	state.SetFileName(L"Setup.ini");

	//몇 번째 프로세스인지 확인
	CString strSection = _T("");
	strSection.Format(_T("SI_INFO_CLIENT"));

	if (state.ReadState(strSection, L"IP", sTemp))
	{
		strcpy_s(CCommonState::Instance()->m_szServerIP, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
	}
	else
	{
		Log::Trace("Section : %s Key : IP 정보를 얻는 데에 실패했습니다.", CCommonFunc::WCharToChar(strSection.GetBuffer(0)));
		return FALSE;
	}

	if (state.ReadState(strSection, L"PORT", nTemp))
	{
		CCommonState::Instance()->m_nPort = nTemp;
	}
	else
	{
		Log::Trace("Section : %s Key : PORT 정보를 얻는 데에 실패했습니다.", CCommonFunc::WCharToChar(strSection.GetBuffer(0)));
		return FALSE;
	}

	if (state.ReadState(strSection, L"ID", sTemp))
	{
		strcpy_s(CCommonState::Instance()->m_szID, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
	}
	else
	{
		Log::Trace("Section : %s Key : ID 정보를 얻는 데에 실패했습니다.", CCommonFunc::WCharToChar(strSection.GetBuffer(0)));
		return FALSE;
	}

	if (state.ReadState(strSection, L"PW", sTemp))
	{
		strcpy_s(CCommonState::Instance()->m_szPass, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
	}
	else
	{
		Log::Trace("Section : %s Key : PW 정보를 얻는 데에 실패했습니다.", strSection);
		return FALSE;
	}

	return TRUE;
}


void CGFSClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == 3)
	{
		KillTimer(3);
		//RegistryTrayIcon();
		OnBnClickedButtonStart();
	}
	else if (nIDEvent == 99)
	{
		if (GetTickCount() - CCommonState::Instance()->m_dwLastRecv >= TIMER_GFS_SERVER_KEEP_ALIVE_PERIOD) {
			CClientInterface::Instance()->GFSProcessRequestKeepAlive();
		}
		if (GetTickCount() - CCommonState::Instance()->m_dwLastRecv > TIMER_GFS_SERVER_KEEP_ALIVE_PERIOD * 2) {

			Log::Trace("Keep Alive Response가 너무 늦었습니다. 프로그램을 종료합니다.");

			m_bKilled = TRUE;

			AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CGFSClientDlg::OnBnClickedButtonStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(false);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(true);

	m_bStarted = true;
}


void CGFSClientDlg::OnBnClickedButtonStop()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);

	m_bStarted = false;
}


void CGFSClientDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.

	if (!m_bKilled) {
		CCommonFunc::KillProcess(L"GFSClient.exe");
	}

	KillTimer(99);

	m_bKilled = TRUE;

	OnBnClickedButtonStop();

	CClientInterface::Instance()->TryShutdown();
	CClientInterface::Instance()->Destroy();
	CClientInterface::Delete();

	CCommonState::Delete();

	Log::Cleanup();
}


BOOL CGFSClientDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

#define		LIMIT_RICH_LOG		(1024*1024*2)
LRESULT CGFSClientDlg::OnLogMessage(WPARAM wParam, LPARAM lParam)
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