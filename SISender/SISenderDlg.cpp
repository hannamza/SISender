
// SISenderDlg.cpp : 구현 파일
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

	int nBufPos = 0;
	BYTE* pData;
	BYTE ringBuffer[EVENT_MAX_COUNT][SI_EVENT_BUF_SIZE];
	BOOL bOverflow = FALSE;

	pDlg->m_nSMIndex = CSM::ReadEventIndex();

	while (pDlg->m_bSMCheck)
	{
		if (pDlg->CheckSMIndexChanged())
		{
			int nEventCount = 0;
			nEventCount = CSM::shm->es.SMIndex - pDlg->m_nSMIndex;
			if (nEventCount < 0)
			{
				nEventCount = EVENT_MAX_COUNT - pDlg->m_nSMIndex;
				bOverflow = TRUE;
			}

			for (int i = 0; i < nEventCount; i++)
			{
				pDlg->m_nSMIndex++;	

				if (nBufPos == EVENT_MAX_COUNT)
				{
					nBufPos = 0;
				}

				pData = ringBuffer[nBufPos];
				CSM::ReadEventBufFromSharedMemory(pDlg->m_nSMIndex, pData, SI_EVENT_BUF_SIZE);
				nBufPos++;
				CEventSend::Instance()->SendEvent(pData);
				Log::Trace("이벤트 들어옴! 공유메모리 Index : %d, 프로그램 버퍼 Index : %d", CSM::shm->es.SMIndex, pDlg->m_nSMIndex);
			}

			if (bOverflow)
			{
				pDlg->m_nSMIndex = -1;
				bOverflow = FALSE;
				Log::Trace("프로그램 이벤트 버퍼 오버플로우 발생! 이벤트 버퍼 인덱스 -1로 초기화!");
			}
		}
	}

	return 0;
}


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


// CSISenderDlg 대화 상자



CSISenderDlg::CSISenderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SISENDER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_bInit = false;
	m_bKilled = false;
	m_bAutoLogin = false;

	m_nAliveCount = 0;
	m_nSMIndex = -1;
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
	ON_MESSAGE(WM_COMMAX_EVENT_PROCESS, &CSISenderDlg::OnCommaxEventProcess)
END_MESSAGE_MAP()


// CSISenderDlg 메시지 처리기

BOOL CSISenderDlg::OnInitDialog()
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

	//자신이 몇번째 SI_SENDER인지를 설정
#ifndef TEST_MODE_FLAG
	CCommonState::Instance()->m_nProcessIndex = _ttoi(theApp.m_lpCmdLine);
#else
	CCommonState::Instance()->m_nProcessIndex = 1;
#endif

	CString strLogFolderName = _T("");
	strLogFolderName.Format(_T("SISender%d_Log"), CCommonState::Instance()->m_nProcessIndex);
	Log::Setup(strLogFolderName);
	Log::SetMainPointer(this);

	Log::Trace("SISender started...");
	Log::Trace("이 프로그램은 SISender%d 입니다.", CCommonState::Instance()->m_nProcessIndex);

	BOOL bSMSucceed = FALSE;
	bSMSucceed = CSM::CreateSharedMemory();
	if (!bSMSucceed)
	{
		Log::Trace("공유메모리 생성에 실패했습니다. 프로그램을 종료합니다.");
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	Log::Trace("공유메모리 생성!");

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

	//INI파일 읽어서 업체 정보에 맞도록 통신 설정
	BOOL bReadIni = FALSE;
	bReadIni = ReadIniFile();
	if (!bReadIni)
	{
		Log::Trace("INI 파일 읽기에 실패했습니다. 프로그램을 종료합니다.");
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	//어떤 SI업체인지 표시
	switch (CCommonState::Instance()->m_nSIType)
	{
	case SI_TEST:
	{
		Log::Trace("이 프로그램은 [TEST]와 통신합니다.");
		break;
	}
	case KOCOM:
	{
		Log::Trace("이 프로그램은 [KOCOM]과 통신합니다.");
		break;
	}
	case COMMAX:
	{
		Log::Trace("이 프로그램은 [COMMAX]와 통신합니다.");
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
			//로그 쓰기 전에 프로그램 종료되어 PM쪽에 위치정보파일이 있는지 여부를 남김
			//Log::Trace("회로 위치 정보 파일을 읽는 데에 실패했습니다. 프로그램을 종료합니다.");
			AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
			return FALSE;
		}

		CCircuitLocInfo::Instance()->GetCircuitLocInfoFromTxtFile(strArr);

		if (CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.size() == 0)
		{
			//로그 쓰기 전에 프로그램 종료되어 PM쪽에 위치정보파일이 있는지 여부를 남김
			//Log::Trace("회로 위치 정보를 읽는 데에 실패했습니다. 프로그램을 종료합니다.");
			AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
			return FALSE;
		}
	}
	
	CClientInterface::New();

	//Commax는 연결 지향이 아님
	if (CCommonState::Instance()->m_nSIType != COMMAX)
	{
		CClientInterface::Instance()->TryConnection(CCommonState::Instance()->m_szServerIP, CCommonState::Instance()->m_nPort);	// 20230111 GBM - INI에 기술된 외부업체 PORT로 들어가야 함
	}
	else
	{
		CCommaxFunc::New();
	}
	
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(true);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(false);

	//INI 성공적으로 읽으면 실행
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

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
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

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CSISenderDlg::OnPaint()
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
HCURSOR CSISenderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSISenderDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	if (nIDEvent == 2)
	{
		KillTimer(2);
		Log::Trace("시작 실패(Serial 연결 실패) - 중지 완료");
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
			if (dw - CCommonState::Instance()->m_dwLastRecv > 60 * 1000)	// 주기가 50초이므로 가장 마지막 keep alive ack 시간과 10초 마진을 보고 그 이상이면 상태이상으로 판단
			{
				//Keep Alive Ack가 늦게 도착했거나 애초에 통신이 맺여지지 않을 경우이므로 프로그램 종료
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
			Log::Trace("PM에 의한 종료!");
			PostMessageW(WM_QUIT);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CSISenderDlg::OnBnClickedButtonStart()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_bStarted)
	{
		return;
	}
	GetDlgItem(IDC_BUTTON_TEST)->EnableWindow(false);

	Log::Trace("Sender start...");

	// 20230111 GBM - FiCon에서 받아온 건물-회로 구성 정보가 적용되어야 함
	CDeviceInfo::New();
	CDeviceInfo::Instance()->SetDeviceInfo();

	G_hShutdown_Event = CreateEvent(NULL, TRUE, FALSE, NULL);

	CEventSend::New();
	CEventSend::Instance()->SetMainWnd(this);
	CEventSend::Instance()->InitEvent();		// 20230111 GBM - 공유메모리에서 얻어진 큐를 처리하는 루틴

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
// 		Log::Trace("시작 실패(Serial 연결 실패) - 중지 중.....");
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
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
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

	BOOL bRet = ::Shell_NotifyIcon(NIM_ADD, &nid); //트레이 아이콘 등록
	ShowWindow(SW_HIDE); //윈도우 감추기
}

DWORD g_dwErrorTime = 0;
LRESULT CSISenderDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	if (wParam == 999 && lParam == 999) { // 연결 에러
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
		CMenu *pMenu = menu.GetSubMenu(0); //활성화 할 메뉴 지정
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

		BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //트레이아이콘 제거
		if (!bRet)
		{
			Log::Trace("트레이 아이콘 제거 실패");
			return -1;
		}
		ShowWindow(SW_SHOW); //윈도우 활성화
		ShowWindow(SW_RESTORE);
	}
	break;
	}
	return 0;
}

void CSISenderDlg::OnMenuStart()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	OnBnClickedButtonStart();
}


void CSISenderDlg::OnMenuStop()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	OnBnClickedButtonStop();
}


void CSISenderDlg::OnMenuPopup()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //트레이아이콘 제거
	if (!bRet)
	{
		Log::Trace("트레이 아이콘 제거 실패");
		return;
	}
	ShowWindow(SW_SHOW); //윈도우 활성화
	ShowWindow(SW_RESTORE);
}


void CSISenderDlg::OnMenuExit()
{
	// TODO: 여기에 명령 처리기 코드를 추가합니다.
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //트레이아이콘 제거

	OnOK();
}


BOOL CSISenderDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CSISenderDlg::OnBnClickedButtonTest()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

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

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	if (!m_bKilled) {
		CCommonFunc::KillProcess(L"Sender 프로그램 실행.exe");
	}

	m_bKilled = true;

	OnBnClickedButtonStop();


	CClientInterface::Instance()->TryShutdown();	
	CClientInterface::Instance()->Destroy();
	CClientInterface::Delete();

	if (CCommonState::Instance()->m_nSIType == COMMAX)
	{
		CCommaxFunc::Delete();
	}

	CCommonState::Delete();

	CCircuitLocInfo::Delete();

	Log::Cleanup();

	//
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //트레이아이콘 제거
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

	//몇 번째 프로세스인지 확인
	CString strSection = _T("");
	strSection.Format(_T("SI_INFO_%d"), CCommonState::Instance()->m_nProcessIndex);

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

	//SI 업체 TYPE 별로 분기
	if (state.ReadState(strSection, L"TYPE", nTemp))
	{
		CCommonState::Instance()->m_nSIType = nTemp;

		switch (nTemp)
		{
		case SI_TEST:
		{
			if (state.ReadState(strSection, L"ID", sTemp))
			{
				strcpy_s(CCommonState::Instance()->m_szID, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
			}
			else
			{
				Log::Trace("Section : %s Key : ID 정보를 얻는 데에 실패했습니다.", CCommonFunc::WCharToChar(strSection.GetBuffer(0)));
				return FALSE;
			}

			if (state.ReadState(strSection, L"PW", sTemp))	//SI_TEST는 PW가 비울 수 있음
			{
				strcpy_s(CCommonState::Instance()->m_szPass, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));
			}
			else
			{
				sTemp = _T("");
				strcpy_s(CCommonState::Instance()->m_szPass, CCommonFunc::WCharToChar(sTemp.GetBuffer(0)));

// 				Log::Trace("Section : %s Key : PW 정보를 얻는 데에 실패했습니다.", strSection);
// 				return FALSE;
			}

			//SI_TEST일 경우 추가 부분

			if (state.ReadState(L"INFO", L"LOGIN", nTemp) && nTemp == 1)	// 20230111 GBM - INI에 기술된 외부업체 서버 접속 ID / PW로 들어가야 함
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
				Log::Trace("SI_TEST - Section : INFO Key : LOGIN 정보를 얻는 데에 실패했습니다.", CCommonFunc::WCharToChar(strSection.GetBuffer(0)));
				return FALSE;
			}

			break;
		}
		case KOCOM:
		{
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
				Log::Trace("Section : %s Key : PW 정보를 얻는 데에 실패했습니다.", CCommonFunc::WCharToChar(strSection.GetBuffer(0)));
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
		Log::Trace("Section : %s, Key : TYPE 정보를 얻는 데에 실패했습니다.", CCommonFunc::WCharToChar(strSection.GetBuffer(0)));
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
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
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

BOOL CSISenderDlg::CheckSMIndexChanged()
{
	BOOL bRet = FALSE;
	if (m_nSMIndex != CSM::shm->es.SMIndex)
		bRet = TRUE;

	return bRet;
}

//
// int MakeSocketNonBlocking(int sock)
// {
// 	unsigned long arg = 1;
// 	return ioctlsocket(sock, FIONBIO, &arg) == 0;
// }
//

LRESULT CSISenderDlg::OnCommaxEventProcess(WPARAM wParam, LPARAM lParam)
{
	// 1: Commax Connect만 비동기처리, 0: Commax 동기 소켓
#if 1
	BYTE* pData = (BYTE*)wParam;

	CCommaxFunc::Instance()->CommaxEventProcess(pData);

	return 0;
#else
	BYTE* pData = (BYTE*)wParam;

	//
	//버퍼 가공
	//화재 타입 정의
	CString strFireType = _T("");
	int nFireType = 0;
	if (pData[SI_EVENT_BUF_COMMAND] == 'R')
	{
		strFireType = _T("화재 일괄 해제");
		nFireType = COMMAX_FIRE_ALARM_ALL_CLEAR;
	}
	//else if((pData[SI_EVENT_BUF_COMMAND] == 'F') && (pData[SI_EVENT_BUF_FIRE_RECEIVER_1] == '0'))		//김호 마스터 확인 결과 두번째 조건을 필요 없음
	else if (pData[SI_EVENT_BUF_COMMAND] == 'F')
	{
		if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'N')
		{
			strFireType = _T("화재 발생");
			nFireType = COMMAX_FIRE_ALARM_FIRE_OCCURED;
		}
		else if (pData[SI_EVENT_BUF_OCCUR_INFO] == 'F')
		{
			strFireType = _T("화재 해제");
			nFireType = COMMAX_FIRE_ALARM_FIRE_CLEAR;
		}
		else
		{
			Log::Trace("화재 타입이 아닌 이벤트가 들어왔습니다. COMMAX 이벤트 전송을 하지 않습니다.");
			return 0;		// N이나 F가 아니면 리턴
		}
	}
	else
	{
		Log::Trace("화재 타입이나 수신기 복구 이외의 이벤트가 들어왔습니다. COMMAX 이벤트 전송을 하지 않습니다.");
		return 0;		//화재 타입이나 수신기 복구가 아니면 리턴
	}

	//건물 정보 - 회로 정보 매칭을 통해 위치 정보 확인
	int nDong = 0;
	int nFloor = 0;
	int nStair = 0;
	int nFloorType = FLOOR_TYPE_ETC;

	//회로번호와 건물정보 매칭
	CString strCircuitNo = _T("");
	strCircuitNo = CCircuitLocInfo::Instance()->GetCircuitNo(pData);

	std::map<CString, CIRCUIT_LOC_INFO>::iterator iter;
	CIRCUIT_LOC_INFO cli;
	//수신기 복구는 위치정보에 없음, 
	if (strCircuitNo.Compare(_T("00000000")) != 0)
	{
		iter = CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.find(strCircuitNo);

		//위치정보에서 찾으면 
		if (iter != CCircuitLocInfo::Instance()->m_mapCircuitLocInfo.end())
		{
			cli = iter->second;
			if (nFireType != COMMAX_FIRE_ALARM_ALL_CLEAR)
			{
				nDong = atoi(cli.buildingName);
			}

			//일단 아파트 건물(101동, 102동 등의 건물이 아닌 주차장 등의 건물은 전송안함)
			if (nDong == 0)
			{
				Log::Trace("아파트 건물 외의 기타 건물의 화재 정보가 들어왔습니다. COMMAX 이벤트 전송을 하지 않습니다.");
				return 0;
			}

			nFloorType = CCircuitLocInfo::Instance()->CheckFloorType(cli.floor);

			if (nFloorType == FLOOR_TYPE_BASEMENT)
			{
				nFloor = atoi(&cli.floor[1]);	// B 이후 숫자 
				nFloor *= -1;
			}
			else if (nFloorType == FLOOR_TYPE_PH)	// COMMAX는 층이 큰 의미가 없고 옥탑층 개념이 없지만 일단 옥탑층 정보를 얻도록 함
			{
				nFloor = atoi(&cli.floor[2]);	// PH 이후 숫자
			}
			else if (nFloorType == FLOOR_TYPE_M)
			{
				nFloor = atoi(&cli.floor[1]);	// M 이후 숫자 
			}
			else // 일반층은 그대로 층수 얻고, RF는 어차피 숫자가 없음, 현재 COMMAX 프로토콜로 RF 표현 불가능
			{
				nFloor = atoi(cli.floor);
			}

			nStair = atoi(cli.stair);

			//호(실) 정보는 COMMAX 프로토콜에 없음
		}

	}
	else //수신기 복구
	{

	}

	CString strBuf = _T("");
	CString strDong = _T("");
	CString strFloor = _T("");
	CString strStair = _T("");

	if (nFireType != COMMAX_FIRE_ALARM_ALL_CLEAR)
	{
		if (nDong != 0)
		{
			strDong.Format(_T("%d"), nDong);
		}

		if (nFloor != 0)
		{
			strFloor.Format(_T("%d"), nFloor);
		}

		if (nStair != 0)
		{
			strStair.Format(_T("%d"), nStair);
		}

		strBuf.Format(_T("%s\r\n\t%s\r\n\t\t%s%d%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t%s\r\n%s")
			, g_lpszCommaxTagName[XML_TAG_START_COMMAX]
			, g_lpszCommaxTagName[XML_TAG_START_FIRE]
			, g_lpszCommaxTagName[XML_TAG_START_EMERGENCY]
			, nFireType
			, g_lpszCommaxTagName[XML_TAG_END_EMERGENCY]
			, g_lpszCommaxTagName[XML_TAG_START_DONG]
			, strDong
			, g_lpszCommaxTagName[XML_TAG_END_DONG]
			, g_lpszCommaxTagName[XML_TAG_START_FLOOR]
			, strFloor
			, g_lpszCommaxTagName[XML_TAG_END_FLOOR]
			, g_lpszCommaxTagName[XML_TAG_START_STAIR]
			, strStair
			, g_lpszCommaxTagName[XML_TAG_END_STAIR]
			, g_lpszCommaxTagName[XML_TAG_START_LINE]
			, _T("")
			, g_lpszCommaxTagName[XML_TAG_END_LINE]
			, g_lpszCommaxTagName[XML_TAG_END_FIRE]
			, g_lpszCommaxTagName[XML_TAG_END_COMMAX]
		);
	}
	else // COMMAX에서 수신기 복구를 화재해제(2), 동을 ALL로 보내달라고 요청함
	{
		strBuf.Format(_T("%s\r\n\t%s\r\n\t\t%s%d%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t\t%s%s%s\r\n\t%s\r\n%s")
			, g_lpszCommaxTagName[XML_TAG_START_COMMAX]
			, g_lpszCommaxTagName[XML_TAG_START_FIRE]
			, g_lpszCommaxTagName[XML_TAG_START_EMERGENCY]
			, COMMAX_FIRE_ALARM_FIRE_CLEAR
			, g_lpszCommaxTagName[XML_TAG_END_EMERGENCY]
			, g_lpszCommaxTagName[XML_TAG_START_DONG]
			, _T("ALL")
			, g_lpszCommaxTagName[XML_TAG_END_DONG]
			, g_lpszCommaxTagName[XML_TAG_START_FLOOR]
			, _T("")
			, g_lpszCommaxTagName[XML_TAG_END_FLOOR]
			, g_lpszCommaxTagName[XML_TAG_START_STAIR]
			, _T("")
			, g_lpszCommaxTagName[XML_TAG_END_STAIR]
			, g_lpszCommaxTagName[XML_TAG_START_LINE]
			, _T("")
			, g_lpszCommaxTagName[XML_TAG_END_LINE]
			, g_lpszCommaxTagName[XML_TAG_END_FIRE]
			, g_lpszCommaxTagName[XML_TAG_END_COMMAX]
		);
	}
	//
	
	sockaddr_in ServerAddr;
	memset(&ServerAddr, 0, sizeof(ServerAddr));
	//ServerAddr.sin_addr.S_un.S_addr = ADDR_ANY;
	
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(CCommonState::Instance()->m_nPort);

	inet_pton(AF_INET, CCommonState::Instance()->m_szServerIP, &(ServerAddr.sin_addr));

	SOCKET commaxSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (commaxSock == INVALID_SOCKET)
	{
		Log::Trace("Commax Socket Creation Failed!");
		return 0;
	}

	//Log::Trace("Commax Socket Creation Succeeded!");

	if (connect(commaxSock, (sockaddr*)&ServerAddr, sizeof(sockaddr_in)) == SOCKET_ERROR)
	{
		DWORD dw;
		dw = GetLastError();
		Log::Trace("Commax Socket Connection Failed!");
		Log::Trace("GetLastError : %d", dw);
		return 0;
	}

	Log::Trace("Commax Socket Connection Succeeded!");

	//


	//send(commaxSock, (char*)pData, SI_EVENT_BUF_SIZE, 0);

// 	char cBuf[300];
// 	memset(cBuf, NULL, 300);
// 	strcpy(cBuf, CCommonFunc::WCharToChar(strBuf.GetBuffer(0)));
// 	int nSendLen = 0;
// 	nSendLen = strBuf.GetLength();
// 	send(commaxSock, cBuf, nSendLen, 0);

	char strUtf8[300];
	memset(strUtf8, NULL, 300);
	strcpy_s(strUtf8, CCommonFunc::WcharToUtf8(strBuf.GetBuffer(0)));
	if (send(commaxSock, strUtf8, strlen(strUtf8), 0) == SOCKET_ERROR)
	{
		DWORD dw;
		dw = WSAGetLastError();
		Log::Trace("Commax Sending Failed!");
		Log::Trace("WSAGetLastError : %d", dw);
		closesocket(commaxSock);
		return 0;
	}

	CString strFloorPrefix = _T("");
	CString strMsg = _T("");

	switch (nFloorType)
	{
	case FLOOR_TYPE_BASEMENT:
	{
		nFloor *= -1;
		strFloorPrefix = _T("B");
		break;
	}
	case FLOOR_TYPE_RF:
	{
		strFloorPrefix = _T("R");
		break;
	}
	case FLOOR_TYPE_PH:
	{
		strFloorPrefix = _T("PH");
		break;
	}
	case FLOOR_TYPE_PIT:
	{
		strFloorPrefix = _T("PIT");
		break;
	}
	case FLOOR_TYPE_M:
	{
		strFloorPrefix = _T("M");
		break;
	}
	default:
		break;
	}

	if (nFireType != COMMAX_FIRE_ALARM_ALL_CLEAR)
	{
		strMsg.Format(_T("Commax Sending Succeeded! - [# 이벤트 타입 : %s, # %s 동, # %s%d F, # %d 계단]"), 
			strFireType,
			strDong,
			strFloorPrefix,
			nFloor,
			nStair);		
	}
	else
	{
		strMsg.Format(_T("Commax Sending Succeeded! - [# 이벤트 타입 : %s, # ALL (동)]"), strFireType);
	}

	Log::Trace("%s", CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));

	BOOL bRecv = FALSE;
	LARGE_INTEGER startTime, curTime;
	QueryPerformanceCounter(&startTime);

	while (true)
	{
		BYTE buffer[265] = { 0, };
		int nLength = 0;
		nLength = recv(commaxSock, (char*)buffer, 265, 0);

		if (nLength > 0)
		{
			bRecv = TRUE;
			break;
		}

		QueryPerformanceCounter(&curTime);
		double deltaTime = CCommonFunc::GetPreciseDeltaTime(startTime, curTime);
		if (deltaTime > 3000)	//3초 이내 Response 없으면 기다리지 않고 소켓 종료
		{
			break;
		}
	}

	if (bRecv)
	{
		Log::Trace("Commax Response Received!");
	}
	else
	{
		Log::Trace("Commax Response Does not Received!");
	}

	if (closesocket(commaxSock) == SOCKET_ERROR)
	{
		DWORD dw;
		dw = WSAGetLastError();
		Log::Trace("Commax Socket Closing Failed!");
		Log::Trace("WSAGetLastError : %d", dw);
	}

	//Log::Trace("Commax Socket Closed!");

	return 0;
#endif
}