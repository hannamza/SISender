
// LogRemoverDlg.cpp : ���� ����
//

#include "stdafx.h"
#include "LogRemover.h"
#include "LogRemoverDlg.h"
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


// CLogRemoverDlg ��ȭ ����



CLogRemoverDlg::CLogRemoverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_LOGREMOVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nAliveCount = 0;
	m_bStarted = false;
}

void CLogRemoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG_FOLDER, m_ctrlListLogFolder);
}

BEGIN_MESSAGE_MAP(CLogRemoverDlg, CDialogEx)
	ON_MESSAGE(WM_TRAY_NOTIFICATION, OnTrayNotification)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_START, &CLogRemoverDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CLogRemoverDlg::OnBnClickedButtonStop)
	ON_COMMAND(32772, &CLogRemoverDlg::OnMenuStart)
	ON_COMMAND(32774, &CLogRemoverDlg::OnMenuStop)
	ON_COMMAND(32776, &CLogRemoverDlg::OnMenuPopup)
	ON_COMMAND(32778, &CLogRemoverDlg::OnMenuExit)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

bool g_bThread = false;
void func_thread(CLogRemoverDlg* pDlg, CString sPath, int nTerm)
{
	std::vector<CString>::iterator iter;
	CString sLogPath = _T("");

	DWORD dwLastTime = 0;
	DWORD dwTerm = (60 * 1000) * (60); // 1 �ð�����
	while (g_bThread)
	{
		DWORD ddd = GetTickCount();
		if (GetTickCount() - dwLastTime > dwTerm)
		{
			CTime CurTime = CTime::GetCurrentTime();
			CTime tDel;
			tDel = CurTime - CTimeSpan(nTerm, 0, 0, 0); // ��, ��, ��, ��

			iter = pDlg->m_vecLogFolder.begin();
			for (; iter != pDlg->m_vecLogFolder.end(); iter++)
			{
				sLogPath = sPath + _T('\\') + *iter;
				pDlg->DeleteLogfiles(sLogPath, tDel, 0, 0, 0);
			}

			dwLastTime = GetTickCount();
		}
		Sleep(1);
	}
}

// CLogRemoverDlg �޽��� ó����

BOOL CLogRemoverDlg::OnInitDialog()
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
	BOOL bSMSucceed = FALSE;
	bSMSucceed = CSM::CreateSharedMemory();
	if (!bSMSucceed)
	{
		AfxGetApp()->m_pMainWnd->PostMessageW(WM_QUIT);
		return FALSE;
	}

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);

	SetDlgItemText(IDC_EDIT_SAVEDAY, _T("30"));

	m_Thread = NULL;

	GetLogFolderList();

	InitControl();

	SetTimer(1, 1000, NULL);

	SetTimer(TIMER_ALIVE_COUNT_ID, TIMER_ALIVE_COUNT_PERIOD, NULL);

#ifndef TEST_MODE_FLAG
	SetTimer(TIMER_PM_QUIT_CHECK_ID, TIMER_PM_QUIT_CHECK_PERIOD, NULL);	//20230118 GBM - test
#endif

	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
}

void CLogRemoverDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CLogRemoverDlg::OnPaint()
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
HCURSOR CLogRemoverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CLogRemoverDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	if (nIDEvent == 1) {
		KillTimer(1);
		RegistryTrayIcon();
		OnBnClickedButtonStart();
	}
	else if (nIDEvent == TIMER_ALIVE_COUNT_ID)
	{
		if (m_nAliveCount != ULLONG_MAX)
			m_nAliveCount++;
		else
			m_nAliveCount = 0;

		CSM::WriteAliveCountToSharedMemory(LOG_REMOVER, m_nAliveCount);
	}
	else if (nIDEvent == TIMER_PM_QUIT_CHECK_ID)
	{
		bool bProcessRun = false;
		bProcessRun = CSM::ReadProcessRunFromSharedMemory(LOG_REMOVER);

		if (!bProcessRun)
		{
			KillTimer(TIMER_PM_QUIT_CHECK_ID);
			PostMessageW(WM_QUIT);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CLogRemoverDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰��մϴ�.
	OnBnClickedButtonStop();

	//
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	BOOL bRet = ::Shell_NotifyIcon(NIM_DELETE, &nid); //Ʈ���̾����� ����
}


BOOL CLogRemoverDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CLogRemoverDlg::OnBnClickedButtonStart()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_Thread)
	{
		return;
	}

	CString sFolder;

	CString sPath;
	TCHAR szPath[2048];
	GetModuleFileName(NULL, szPath, 2048);
	PathRemoveFileSpec(szPath);

	m_sPath.Format(_T("%s"), szPath);

	GetDlgItemText(IDC_EDIT_SAVEDAY, sFolder);
	int nTerm = _ttoi(sFolder.GetBuffer(0));
	sFolder.ReleaseBuffer();

	g_bThread = true;
	m_Thread = new thread(&func_thread, this, m_sPath, nTerm);

	m_bStarted = true;

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(FALSE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(TRUE);

	GetDlgItem(IDC_EDIT_SAVEDAY)->EnableWindow(FALSE);
}


void CLogRemoverDlg::OnBnClickedButtonStop()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (!m_Thread)
	{
		return;
	}
	g_bThread = false;
	m_Thread->join();

	SAFE_DELETE(m_Thread);

	m_bStarted = false;

	GetDlgItem(IDC_BUTTON_START)->EnableWindow(TRUE);
	GetDlgItem(IDC_BUTTON_STOP)->EnableWindow(FALSE);

	GetDlgItem(IDC_EDIT_SAVEDAY)->EnableWindow(TRUE);
}

BOOL DeleteDirectory(CString lpDirPath)
{
	if (lpDirPath.GetLength() <= 3)// ��ΰ� ���� ��� �ǵ��ư���.
	{
		return FALSE;
	}
	BOOL bRval = FALSE;
	int nRval = 0;
	CString szNextDirPath = _T("");
	CString szRoot = _T("");
	CFileFind find;

	// ������ ���� �ϴ� �� Ȯ�� �˻�
	bRval = find.FindFile(lpDirPath);
	if (bRval == FALSE)
	{
		return bRval;
	}

	while (bRval)
	{
		bRval = find.FindNextFile();
		// . or .. �� ��� ���� �Ѵ�.
		if (find.IsDots() == TRUE)
		{
			continue;
		}
		// Directory �� ���
		if (find.IsDirectory())
		{
			szNextDirPath.Format(_T("%s\\*.*"), find.GetFilePath());
			// Recursion function ȣ ��           
			DeleteDirectory(szNextDirPath);
		}
		// file�� ���          
		else
		{
			// ���� ����
			::DeleteFile(find.GetFilePath());
		}
	}
	szRoot = find.GetRoot();
	find.Close();
	bRval = RemoveDirectory(szRoot);
	return bRval;
}

void CLogRemoverDlg::DeleteLogfiles(CString sPath, CTime & tDel, int nYear, int nMon, int nType)
{
	CString sYear, sMon, sDay, sTemp, sValue;
	int nDay = 0;
	CFileFind finder;
	BOOL bRes;
	int nLogTime, nDelTime;
	int nTemp = 0;
	sTemp.Format(_T("%s\\*.*"), sPath);
	bRes = finder.FindFile(sTemp);
	while (bRes)
	{
		bRes = finder.FindNextFile();
		if (finder.IsDots())
		{
			continue;
		}
		sValue = finder.GetFileName();
		nTemp = _ttoi(sValue.GetBuffer(0));
		if (finder.IsDirectory()) // ����
		{
			sValue = finder.GetFileName();
			nTemp = _ttoi(sValue.GetBuffer(0));
			if (nTemp == 0)
			{
				continue;
			}
			switch (nType)
			{
			case 0: // year
				sYear = finder.GetFileName();
				nYear = _ttoi(sYear.GetBuffer(0));
				sTemp.Format(_T("%s\\%s"), sPath, sYear);
				DeleteLogfiles(sTemp, tDel, nYear, 0, 1);
				break;
			case 1: // month
				sMon = finder.GetFileName();
				nMon = _ttoi(sMon.GetBuffer(0));
				sTemp.Format(_T("%s\\%s"), sPath, sMon);
				DeleteLogfiles(sTemp, tDel, nYear, nMon, 2);
				break;
			case 2: // day
				sDay = finder.GetFileName();
				nDay = _ttoi(sDay.GetBuffer(0));
				sValue.Format(_T("%s\\%s"), sPath, sDay);
				break;
			}

			sTemp.Format(_T("%04d%02d%02d"), nYear, nMon, nDay);
			nLogTime = _ttoi(sTemp.GetBuffer(0));
			sTemp.Format(_T("%04d%02d%02d"), tDel.GetYear(), tDel.GetMonth(), tDel.GetDay());
			nDelTime = _ttoi(sTemp.GetBuffer(0));

			if (nLogTime <= nDelTime)
			{
				// delete
				DeleteDirectory(sValue);
			}
		}
	}
}

void CLogRemoverDlg::GetLogFolderList()
{
	m_vecLogFolder.push_back(_T("Broker_Log"));

	INT nTemp = 0;
	CReadWriteState state;
	CString sTemp;
	state.SetFileName(L"Setup.ini");

	CString strSection = _T("");
	for (int i = SI_SENDER1; i < PROCESS_TOTAL_COUNT; i++)
	{
		strSection.Format(_T("SI_INFO_%d"), i);

		if (!state.ReadState(strSection, L"IP", sTemp))
		{
			continue;
		}

		if (!state.ReadState(strSection, L"PORT", nTemp))
		{
			continue;
		}

		if (!state.ReadState(strSection, L"ID", sTemp))
		{
			continue;
		}

		//SI ��ü TYPE ���� �б�
		if (state.ReadState(strSection, L"TYPE", nTemp))
		{
			switch (nTemp)
			{
			case SI_TEST:
			{
				if (!state.ReadState(L"INFO", L"LOGIN", nTemp) && nTemp == 1)
				{
					continue;
				}

				break;
			}
			case KOCOM:
			{
				if (!state.ReadState(strSection, L"PW", sTemp))
				{
					continue;;
				}

				break;
			}
			default:
			{
				continue;
			}
			}
		}
		else
		{
			continue;
		}

		sTemp.Format(_T("SISender%d_Log"), i);
		m_vecLogFolder.push_back(sTemp);
	}

	m_vecLogFolder.push_back(_T("PM_Log"));
}

void CLogRemoverDlg::InitControl()
{
	m_ctrlListLogFolder.InsertColumn(0, _T("No."), LVCFMT_CENTER, 50);
	m_ctrlListLogFolder.InsertColumn(1, _T("���� �α� �j����"), LVCFMT_CENTER, 150);

	m_ctrlListLogFolder.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	m_ctrlListLogFolder.ModifyStyle(LVS_TYPEMASK, LVS_REPORT);

	std::vector<CString>::iterator iter;
	iter = m_vecLogFolder.begin();

	int nIndex = 0;
	CString sTemp;
	for (; iter != m_vecLogFolder.end(); iter++)
	{
		sTemp.Format(_T("%d"), nIndex + 1);
		m_ctrlListLogFolder.InsertItem(nIndex, sTemp);
		m_ctrlListLogFolder.SetItem(nIndex, 1, LVIF_TEXT, *iter, NULL, NULL, NULL, NULL);

		nIndex++;
	}
}

DWORD g_dwErrorTime = 0;
LRESULT CLogRemoverDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
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
			AfxMessageBox(_T("Ʈ���� ������ ���� ����"));
			return -1;
		}
		ShowWindow(SW_SHOW); //������ Ȱ��ȭ
		ShowWindow(SW_RESTORE);
	}
	break;
	}
	return 0;
}

void CLogRemoverDlg::OnMenuStart()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	OnBnClickedButtonStart();
}


void CLogRemoverDlg::OnMenuStop()
{
	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
	OnBnClickedButtonStop();
}


void CLogRemoverDlg::OnMenuPopup()
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
		AfxMessageBox(_T("Ʈ���� ������ ���� ����"));
		return;
	}
	ShowWindow(SW_SHOW); //������ Ȱ��ȭ
	ShowWindow(SW_RESTORE);
}


void CLogRemoverDlg::OnMenuExit()
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

void CLogRemoverDlg::RegistryTrayIcon()
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.uID = 0;
	nid.hWnd = GetSafeHwnd();

	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
	nid.hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	_tcscpy(nid.szTip, L"LogRemover");
	nid.uCallbackMessage = WM_TRAY_NOTIFICATION;

	BOOL bRet = ::Shell_NotifyIcon(NIM_ADD, &nid); //Ʈ���� ������ ���
	ShowWindow(SW_HIDE); //������ ���߱�
}

void CLogRemoverDlg::OnClose()
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.
	RegistryTrayIcon();
	return;

	CDialogEx::OnClose();
}
