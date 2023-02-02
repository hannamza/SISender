
// PMDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "PM.h"
#include "PMDlg.h"
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


// CPMDlg ��ȭ ����



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


// CPMDlg �޽��� ó����

BOOL CPMDlg::OnInitDialog()
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

	//���α׷� ���� ǥ��
	ShowProgramVersion();

	Log::Setup();
	Log::SetMainPointer(this);

	Log::Trace("PM Start!");

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
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

	BOOL bReadINI = FALSE;
 	bReadINI = InitProcessInfo();
	if (!bReadINI)
	{
		Log::Trace("INI ���� ������ �д� ���� �����߽��ϴ�. ���α׷��� �����մϴ�.");
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	Log::Trace("INI ���� �ε� �Ϸ�!");

	InitControl();

	//QuitGracefullyAllExe();

 	KillAllExe();

	BOOL bExcuteComplete = FALSE;
	bExcuteComplete = ExcuteAllExe();
	if (!bExcuteComplete)
	{
		Log::Trace("���μ��� ���࿡ ������ ���α׷��� �����մϴ�.");
		AfxMessageBox(_T("���μ��� ���࿡ ������ ���α׷��� �����մϴ�."));
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	Log::Trace("�� ���μ����� ���� üũ�� �����մϴ�.");
	SetTimer(TIMER_PROCESS_CHECK_ID, TIMER_PROCESS_CHECK_PERIOD, NULL);

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CPMDlg::OnPaint()
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
HCURSOR CPMDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CPMDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
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

	strVersionInfo.Format(_T("Program Ver : [%s] ���� ���� Protocol : [%s]"), strProgramVersion, strProtocol);
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
				//�ּ� �ϳ��� SI_SENDER ������ �־�� �ϹǷ� ù��°�� TYPE������ �� �������� ini�� ��ã�� ������ �Ǵ��ϰ� ���α׷� ����
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
				if (pPInfo->SIType != SI_TEST)	// ���� GFSM�� PW�� ����
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
			Log::Trace("[%s] ���� ���μ��� ����!", CCommonFunc::WCharToChar(strProcessNameOnly.GetBuffer(0)));
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

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.

	if(!m_bDupExecute)
		QuitGracefullyAllExe();

	ReleaseProcessInfo();

	CSM::CloseSharedMemory();

	Log::Trace("�����޸� ����!");

	Log::Trace("PM�� �����մϴ�.");

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
	m_ctrlListProcess.InsertColumn(1, _T("���μ�����"), LVCFMT_CENTER, 150);
	m_ctrlListProcess.InsertColumn(2, _T("SI��ü"), LVCFMT_CENTER, 150);
	m_ctrlListProcess.InsertColumn(3, _T("��������"), LVCFMT_CENTER, 100);

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
				strMsg.Format(_T("[%s] ���࿡ �����߽��ϴ�."), CCommonFunc::GetFileNameOnly((*iter)->processName));
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
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
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
				// ���̰�
				if ((*iter)->processId != 0)
				{
					m_pw.SoftKillProcess((*iter)->processId);
				}
				else
				{
					//���� ������ �� processID�� ��´ٴ� ������ �����ϹǷ� �� ��ƾ�� ó������ ����

					//���� ���μ��� ���� ������ �� ���� ������ �������� SISender�߿� use�� �͵� �� �ϳ��� ������ ����� �ϴ� �� ���̱� ������ ����(���μ������� ���� ����)
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

				// �ٽ� �츲
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

				//���μ��� ���̵� �������� ���츰 �ɷ� �Ǵ�
				if ((*iter)->processId != 0)
				{
					bAlive = true;
					Log::Trace("[%s] PM�� ���� ����� ����", CCommonFunc::WCharToChar(strProcessNameOnly.GetBuffer(0)));
				}
				else
				{
					bAlive = false;
					Log::Trace("[%s] PM�� ���� ����� ����", CCommonFunc::WCharToChar(strProcessNameOnly.GetBuffer(0)));
				}
				//
			}

			//���� �޶����� ����Ʈ ����
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
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (AfxMessageBox(_T("������ PM�� �����Ͻðڽ��ϱ�?"), MB_YESNO | MB_ICONWARNING) == IDNO)
	{
		return;
	}

	CDialogEx::OnClose();
}
