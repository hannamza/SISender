
// PMDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "PM.h"
#include "PMDlg.h"
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


// CPMDlg 대화 상자



CPMDlg::CPMDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	memset(m_nPreAliveCount, 0, sizeof(ULONGLONG) * PROCESS_TOTAL_COUNT);
	m_bDupExecute = false;
}

void CPMDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PROCESS, m_ctrlListProcess);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_richLog);
	DDX_Control(pDX, IDC_CHECK_SCROLL, m_checkScroll);
}

BEGIN_MESSAGE_MAP(CPMDlg, CDialogEx)
	ON_MESSAGE(LIST_MESSAGE, OnListMessage)
	ON_MESSAGE(LOG_MESSAGE, OnLogMessage)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_CLOSE()
END_MESSAGE_MAP()


// CPMDlg 메시지 처리기

BOOL CPMDlg::OnInitDialog()
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

	//프로그램 버전 표시
	ShowProgramVersion();

	Log::Setup();
	Log::SetMainPointer(this);

	Log::Trace("PM Start!");

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
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

	BOOL bReadINI = FALSE;
 	bReadINI = InitProcessInfo();
	if (!bReadINI)
	{
		Log::Trace("INI 파일 설정을 읽는 데에 실패했습니다. 프로그램을 종료합니다.");
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	Log::Trace("INI 설정 로드 완료!");

	InitControl();

	//QuitGracefullyAllExe();

 	KillAllExe();

	BOOL bExcuteComplete = FALSE;
	bExcuteComplete = ExcuteAllExe();
	if (!bExcuteComplete)
	{
		Log::Trace("프로세스 실행에 실패해 프로그램을 종료합니다.");
		AfxMessageBox(_T("프로세스 실행에 실패해 프로그램을 종료합니다."));
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	Log::Trace("각 프로세스의 상태 체크를 시작합니다.");
	SetTimer(TIMER_PROCESS_CHECK_ID, TIMER_PROCESS_CHECK_PERIOD, NULL);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CPMDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CPMDlg::OnPaint()
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
HCURSOR CPMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CPMDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

void CPMDlg::ShowProgramVersion()
{
	CString strVersionInfo = _T("");
	CString strProgramVersion = _T("");

	strProgramVersion = CCommonFunc::GetProgramVersion();

	CString strProtocol = _T("");
	for (int nSI = 0; nSI < SI_TOTAL_COUNT; nSI++)
	{
		if (nSI == SI_TEST)
			continue;

		CString strTemp = _T("");
		strTemp.Format(_T("%s"), g_lpszSIName[nSI]);

		strProtocol += strTemp;
		if (nSI != SI_TOTAL_COUNT - 1)
			strProtocol += _T(" ");

	}

	strVersionInfo.Format(_T("Program Ver : [%s] 현재 지원 Protocol : [%s]"), strProgramVersion, strProtocol);
	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_PROGRAM_VERSION);
	pStatic->SetWindowTextW(strVersionInfo);
}

BOOL CPMDlg::InitProcessInfo()
{

	INT nTemp = 0;
	CReadWriteState state;
	int nSISenderIndex = 1;
	CString sTemp, strSection, strProcessName;
	state.SetFileName(L"Setup.ini");

	for (int i = 0; i < PROCESS_TOTAL_COUNT; i++)
	{
		strSection = _T("");

		PROCESS_INFO* pPInfo = new PROCESS_INFO;

		pPInfo->use = true;
		pPInfo->SIType = 0;
		pPInfo->processId = 0;

		TCHAR szPath[2048];
		GetModuleFileName(NULL, szPath, 2048);
		PathRemoveFileSpec(szPath);
		strProcessName.Format(_T("%s\\%s"), szPath, g_lpszProcessName[i]);
		_tcscpy(pPInfo->processName, strProcessName.GetBuffer(0));

		switch (i)
		{
		case BROKER:
		{		
			break;
		}
		case SI_SENDER1:
		case SI_SENDER2:
		case SI_SENDER3:
		{
			strSection.Format(_T("SI_INFO_%d"), nSISenderIndex);

			nSISenderIndex++;

			if (state.ReadState(strSection, L"TYPE", nTemp))
			{
				pPInfo->SIType = nTemp;
			}
			else
			{
				//최소 하나의 SI_SENDER 설정이 있어야 하므로 첫번째에 TYPE정보를 못 가져오면 ini를 못찾은 것으로 판단하고 프로그램 종료
				if (i == SI_SENDER1)
				{
					//return FALSE;
				}

				pPInfo->use = false;
				break;
			}

			if (!state.ReadState(strSection, L"IP", sTemp))
			{
				pPInfo->use = false;
				break;
			}

			if (!state.ReadState(strSection, L"PORT", sTemp))
			{
				pPInfo->use = false;
				break;
			}

			if (!state.ReadState(strSection, L"ID", sTemp))
			{
				pPInfo->use = false;
				break;
			}

			if (!state.ReadState(strSection, L"PW", sTemp))
			{
				if (pPInfo->SIType != SI_TEST)	// 기존 GFSM은 PW가 없음
				{
					pPInfo->use = false;
					break;
				}
			}

			break;
		}
		case LOG_REMOVER:
		{
			pPInfo->use = true;
			break;
		}
		default:
			break;
		}

		m_vecProcessInfo.push_back(pPInfo);
	}

	return TRUE;
}

void CPMDlg::KillAllExe()
{
	std::vector<PROCESS_INFO*>::iterator iter;
	iter = m_vecProcessInfo.begin();

	std::vector<DWORD> vecProcessID;
	std::vector<DWORD>::iterator iterPID;
	CString strProcessNameOnly = _T("");
	CString strProcessName = _T("");

	int nCnt = BROKER;
	for (; iter != m_vecProcessInfo.end(); iter++)
	{
		strProcessName.Format(_T("%s"), (*iter)->processName);
		strProcessNameOnly = CCommonFunc::GetFileNameOnly(strProcessName);

		if (nCnt >= SI_SENDER2 && nCnt <= SI_SENDER3)
		{
			nCnt++;
			continue;
		}

		vecProcessID.clear();
		BOOL bProcessIDFound = FALSE;
		bProcessIDFound = m_pw.GetProcessIDsByFileName(strProcessNameOnly.GetBuffer(0), vecProcessID);

		if (bProcessIDFound)
		{
			iterPID = vecProcessID.begin();
			for (; iterPID != vecProcessID.end(); iterPID++)
			{
				m_pw.ForceKillProcess(*iterPID);
			}
			strProcessNameOnly = strProcessNameOnly.Left(strProcessNameOnly.GetLength() - 4);
			Log::Trace("[%s] 기존 프로세스 종료!", CCommonFunc::WCharToChar(strProcessNameOnly.GetBuffer(0)));
		}

		nCnt++;
	}
}

void CPMDlg::ReleaseProcessInfo()
{
	std::vector<PROCESS_INFO*>::iterator iter;
	iter = m_vecProcessInfo.begin();

	for (; iter != m_vecProcessInfo.end(); iter++)
	{
		delete *iter;
	}

	m_vecProcessInfo.clear();
}

void CPMDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 여기에 메시지 처리기 코드를 추가합니다.

	if(!m_bDupExecute)
		QuitGracefullyAllExe();

	ReleaseProcessInfo();

	CSM::CloseSharedMemory();

	Log::Trace("공유메모리 삭제!");

	Log::Trace("PM을 종료합니다.");

	Log::Cleanup();
}

#define		LIMIT_RICH_LOG		(1024*1024*2)
LRESULT CPMDlg::OnLogMessage(WPARAM wParam, LPARAM lParam)
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

void CPMDlg::InitControl()
{
	m_ctrlListProcess.InsertColumn(0, _T("No."), LVCFMT_CENTER, 50);
	m_ctrlListProcess.InsertColumn(1, _T("프로세스명"), LVCFMT_CENTER, 150);
	m_ctrlListProcess.InsertColumn(2, _T("SI업체"), LVCFMT_CENTER, 150);
	m_ctrlListProcess.InsertColumn(3, _T("상태정보"), LVCFMT_CENTER, 100);

	m_ctrlListProcess.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ctrlListProcess.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);

	std::vector<PROCESS_INFO*>::iterator iter;
	iter = m_vecProcessInfo.begin();

	int nIndex = 0;
	int nCnt = 0;
	CString strTemp = _T("");
	for (; iter != m_vecProcessInfo.end(); iter++)
	{
		if ((*iter)->use == false)
		{
			nCnt++;
			continue;
		}

		CString strProcessNameOnly = _T("");		
		strProcessNameOnly = CCommonFunc::GetFileNameOnly((*iter)->processName);

		strTemp.Format(_T("%d"), nIndex + 1);
		m_ctrlListProcess.InsertItem(nIndex, strTemp);
		m_ctrlListProcess.SetItem(nIndex, 1, LVIF_TEXT, strProcessNameOnly, NULL, NULL, NULL, NULL);

		if((nCnt >= SI_SENDER1) && (nCnt <= SI_SENDER3))
			m_ctrlListProcess.SetItem(nIndex, 2, LVIF_TEXT, g_lpszSIName[(*iter)->SIType], NULL, NULL, NULL, NULL);

		m_ctrlListProcess.SetItem(nIndex, 3, LVIF_TEXT, _T("Stopped"), NULL, NULL, NULL, NULL);

		nIndex++;
		nCnt++;
	}

	m_checkScroll.SetCheck(true);
}

BOOL CPMDlg::ExcuteAllExe()
{
	std::vector<PROCESS_INFO*>::iterator iter;
	iter = m_vecProcessInfo.begin();

	int nUse = 0;
	int nCnt = 0;
	for (; iter != m_vecProcessInfo.end(); iter++)
	{
		if ((*iter)->use == true)
		{
			int ret = 0;
			int nProcessId = 0;

			if ((nCnt >= SI_SENDER1) && (nCnt <= SI_SENDER3))
			{
				CString strCommandLine = _T("");
				strCommandLine.Format(_T("%s %d"), (*iter)->processName, nCnt);
				nProcessId = m_pw.RunProcessAndForget(NULL, strCommandLine, &ret);
			}
			else
			{
				nProcessId = m_pw.RunProcessAndForget(NULL, (*iter)->processName, &ret);
			}

			if (nProcessId != 0)
			{
				(*iter)->processId = nProcessId;
				m_ctrlListProcess.SetItem(nUse, 3, LVIF_TEXT, _T("Running"), NULL, NULL, NULL, NULL);
				//PostMessageW(LIST_MESSAGE, (WPARAM)nUse, true);
				CSM::WriteProcessRunToSharedMemory(nCnt, true);
			}
			else
			{
				CString strMsg = _T("");
				strMsg.Format(_T("[%s] 실행에 실패했습니다."), CCommonFunc::GetFileNameOnly((*iter)->processName));
				AfxMessageBox(strMsg);
				Log::Trace(CCommonFunc::WCharToChar(strMsg.GetBuffer(0)));
				return FALSE;
			}

			nUse++;
		}

		nCnt++;
	}

	return TRUE;
}


void CPMDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == TIMER_PROCESS_CHECK_ID)
	{
		std::vector<PROCESS_INFO*>::iterator iter;
		iter = m_vecProcessInfo.begin();

		int nCnt = 0;
		int nListRow = 0;
		for (; iter != m_vecProcessInfo.end(); iter++)
		{
			CString strProcessName = _T("");
			CString strProcessNameOnly = _T("");

			strProcessName.Format(_T("%s"), (*iter)->processName);
			strProcessNameOnly = CCommonFunc::GetFileNameOnly(strProcessName);
			strProcessNameOnly = strProcessNameOnly.Left(strProcessNameOnly.GetLength() - 4);

			if ((*iter)->use == false)
			{
				nCnt++;
				continue;
			}

			bool bAlive = false;
			ULONGLONG nCurAliveCount = 0;
			nCurAliveCount = CSM::ReadAliveCountFromSharedMemory(nCnt);
			if (nCurAliveCount != 0)
			{
				if (m_nPreAliveCount[nCnt] != nCurAliveCount)
				{
					bAlive = true;
				}
				else
				{
					bAlive = false;
				}
			}
			else
			{
				bAlive = false;
			}

			if (!bAlive)
			{
				// 죽이고
				if ((*iter)->processId != 0)
				{
					m_pw.SoftKillProcess((*iter)->processId);
				}
				else
				{
					//애초 시작할 때 processID를 얻는다는 전제로 시작하므로 이 루틴은 처리하지 않음

					//관리 프로세스 들의 루프를 다 돌고 나서는 괜찮지만 SISender중에 use인 것들 중 하나가 문제가 생기면 일단 다 죽이기 문제가 있음(프로세스명이 같기 때문)
// 					std::vector<DWORD> vecProcessID;
// 					std::vector<DWORD>::iterator iterPID;
// 
// 					vecProcessID.clear();
// 					BOOL bProcessIDFound = FALSE;
// 					bProcessIDFound = m_pw.GetProcessIDsByFileName(strProcessNameOnly.GetBuffer(0), vecProcessID);
// 
// 					if (bProcessIDFound)
// 					{
// 						iterPID = vecProcessID.begin();
// 						for (; iterPID != vecProcessID.end(); iterPID++)
// 						{
// 							m_pw.SoftKillProcess(*iterPID);
// 						}
// 					}
				}

				PostMessageW(LIST_MESSAGE, (WPARAM)nListRow, (LPARAM)bAlive);
				CSM::WriteProcessRunToSharedMemory(nCnt, bAlive);

				m_nPreAliveCount[nCnt] = 0;

				// 다시 살림
				int ret = 0;
				if ((nCnt >= SI_SENDER1) && (nCnt <= SI_SENDER3))
				{
					CString strCommandLine = _T("");
					strCommandLine.Format(_T("%s %d"), (*iter)->processName, nCnt);
					(*iter)->processId = m_pw.RunProcessAndForget(NULL, strCommandLine, &ret);
				}
				else
				{
					(*iter)->processId = m_pw.RunProcessAndForget(NULL, (*iter)->processName, &ret);
				}

				if ((nCnt >= SI_SENDER1) && (nCnt <= SI_SENDER3))
				{
					strProcessNameOnly.Format(_T("%s %d"), strProcessNameOnly, nCnt);
				}

				//프로세스 아이디 못받으면 못살린 걸로 판단
				if ((*iter)->processId != 0)
				{
					bAlive = true;
					Log::Trace("[%s] PM에 의해 재시작 성공", CCommonFunc::WCharToChar(strProcessNameOnly.GetBuffer(0)));
				}
				else
				{
					bAlive = false;
					Log::Trace("[%s] PM에 의해 재시작 실패", CCommonFunc::WCharToChar(strProcessNameOnly.GetBuffer(0)));
				}
				//
			}

			//값이 달라지면 리스트 갱신
			if (bAlive != CSM::ReadProcessRunFromSharedMemory(nCnt))
			{
				PostMessageW(LIST_MESSAGE, (WPARAM)nListRow, (LPARAM)bAlive);
				CSM::WriteProcessRunToSharedMemory(nCnt, bAlive);
			}

			m_nPreAliveCount[nCnt] = nCurAliveCount;

			nCnt++;
			nListRow++;
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}

LRESULT CPMDlg::OnListMessage(WPARAM wParam, LPARAM lParam)
{
	int nRow = 0;
	bool bState = 0;
	CString strState = _T("");

	nRow = (int)wParam;
	bState = (bool)lParam;

	if (bState)
	{
		strState = _T("Running");
	}
	else
	{
		strState = _T("Stopped");
	}

	m_ctrlListProcess.SetItem(nRow, 3, LVIF_TEXT, strState, NULL, NULL, NULL, NULL);

	return 0;
}

void CPMDlg::QuitGracefullyAllExe()
{
	std::vector<PROCESS_INFO*>::iterator iter;
	iter = m_vecProcessInfo.begin();

	int nCnt = 0;
	for (; iter != m_vecProcessInfo.end(); iter++)
	{
		if ((*iter)->use == true)
		{
			CSM::WriteProcessRunToSharedMemory(nCnt, false);
			PostMessageW(LIST_MESSAGE, (WPARAM)nCnt, (LPARAM)false);
		}

		nCnt++;
	}
}

void CPMDlg::OnClose()
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (AfxMessageBox(_T("정말로 PM을 종료하시겠습니까?"), MB_YESNO | MB_ICONWARNING) == IDNO)
	{
		return;
	}

	CDialogEx::OnClose();
}
