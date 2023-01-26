// DlgEventTest.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Broker.h"
#include "DlgEventTest.h"
#include "afxdialogex.h"


// CDlgEventTest 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgEventTest, CDialogEx)

CDlgEventTest::CDlgEventTest(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TEST_DIALOG, pParent)
{
	memset(m_eventBuf, NULL, SI_EVENT_BUF_SIZE);
}

CDlgEventTest::~CDlgEventTest()
{
}

void CDlgEventTest::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_COMMAND, m_ctrlComboCommand);
	DDX_Control(pDX, IDC_COMBO_RECEIVER_NO, m_ctrlComboReceiverNo);
	DDX_Control(pDX, IDC_COMBO_UNIT_NO, m_ctrlComboUnitNo);
	DDX_Control(pDX, IDC_COMBO_SYSTEM_NO, m_ctrlComboSystemNo);
	DDX_Control(pDX, IDC_COMBO_CIRCUIT_NO, m_ctrlComboCircuitNo);
	DDX_Control(pDX, IDC_COMBO_OCCUR_INFO, m_ctrlComboOccerInfo);
}


BEGIN_MESSAGE_MAP(CDlgEventTest, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CONFIRM, &CDlgEventTest::OnBnClickedButtonConfirm)
	ON_BN_CLICKED(IDC_BUTTON_CANCEL, &CDlgEventTest::OnBnClickedButtonCancel)
	ON_CBN_SELCHANGE(IDC_COMBO_COMMAND, &CDlgEventTest::OnSelchangeComboCommand)
END_MESSAGE_MAP()


// CDlgEventTest 메시지 처리기입니다.


BOOL CDlgEventTest::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	InitControl();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


BOOL CDlgEventTest::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}


void CDlgEventTest::OnBnClickedButtonConfirm()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	MakeEventBuf();
	
	OnOK();
}


void CDlgEventTest::OnBnClickedButtonCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.

	OnCancel();
}

void CDlgEventTest::InitControl()
{
	CString strItem = _T("");

	for (int nCommand = 0; nCommand < COMMAND_TOTAL_COUNT; nCommand++)
	{		
		strItem.Format(_T("%s"), g_lpszCommandString[nCommand]);
		m_ctrlComboCommand.InsertString(m_ctrlComboCommand.GetCount(), strItem);
	}

	m_ctrlComboCommand.SetCurSel(0);

	for (int nReceiverNo = 0; nReceiverNo < RECEIVER_MAX_COUNT; nReceiverNo++)
	{
		strItem.Format(_T("%d"), nReceiverNo);
		m_ctrlComboReceiverNo.InsertString(m_ctrlComboReceiverNo.GetCount(), strItem);
	}

	m_ctrlComboReceiverNo.SetCurSel(0);

	for (int nUnitNo = 0; nUnitNo < UNIT_MAX_COUNT; nUnitNo++)
	{
		strItem.Format(_T("%d"), nUnitNo);
		m_ctrlComboUnitNo.InsertString(m_ctrlComboUnitNo.GetCount(), strItem);
	}

	m_ctrlComboUnitNo.SetCurSel(0);

	for (int nSystemNo = 0; nSystemNo < SYSTEM_MAX_COUNT; nSystemNo++)
	{
		strItem.Format(_T("%d"), nSystemNo);
		m_ctrlComboSystemNo.InsertString(m_ctrlComboSystemNo.GetCount(), strItem);
	}

	m_ctrlComboSystemNo.SetCurSel(0);

	for (int nCircuitNo = 1; nCircuitNo < CIRCUIT_MAX_COUNT; nCircuitNo++)
	{
		strItem.Format(_T("%d"), nCircuitNo);
		m_ctrlComboCircuitNo.InsertString(m_ctrlComboCircuitNo.GetCount(), strItem);
	}

	m_ctrlComboCircuitNo.SetCurSel(0);

	for (int nOccerInfoNo = 0; nOccerInfoNo < OCCUR_INFO_TOTAL_COUNT; nOccerInfoNo++)
	{
		strItem.Format(_T("%s"), g_lpszOccurInfoString[nOccerInfoNo]);
		m_ctrlComboOccerInfo.InsertString(m_ctrlComboOccerInfo.GetCount(), strItem);
	}

	m_ctrlComboOccerInfo.SetCurSel(0);
}

void CDlgEventTest::MakeEventBuf()
{
	int nIndex = 0;
	CString strTemp = _T("");

	memset(m_eventBuf, NULL, SI_EVENT_BUF_SIZE);

	m_eventBuf[SI_EVENT_BUF_COMPANY_1] = 'G';
	m_eventBuf[SI_EVENT_BUF_COMPANY_2] = 'X';

	nIndex = m_ctrlComboCommand.GetCurSel();
	m_eventBuf[SI_EVENT_BUF_COMMAND] = g_lpszCommand[nIndex];
	
	nIndex = m_ctrlComboReceiverNo.GetCurSel();
	strTemp.Format(_T("%02d"), nIndex);
	m_eventBuf[SI_EVENT_BUF_FIRE_RECEIVER_1] = strTemp.GetAt(0);
	m_eventBuf[SI_EVENT_BUF_FIRE_RECEIVER_2] = strTemp.GetAt(1);
	
	nIndex = m_ctrlComboUnitNo.GetCurSel();
	strTemp.Format(_T("%02d"), nIndex);
	m_eventBuf[SI_EVENT_BUF_UNIT_1] = strTemp.GetAt(0);
	m_eventBuf[SI_EVENT_BUF_UNIT_2] = strTemp.GetAt(1);

	m_eventBuf[SI_EVENT_BUF_DASH] = '-';

	nIndex = m_ctrlComboSystemNo.GetCurSel();
	strTemp.Format(_T("%d"), nIndex);
	m_eventBuf[SI_EVENT_BUF_SYSTEM] = strTemp.GetAt(0);

	nIndex = m_ctrlComboCircuitNo.GetCurSel();
	strTemp.Format(_T("%03d"), nIndex);
	m_eventBuf[SI_EVENT_BUF_CIRCUIT_1] = strTemp.GetAt(0);
	m_eventBuf[SI_EVENT_BUF_CIRCUIT_2] = strTemp.GetAt(1);
	m_eventBuf[SI_EVENT_BUF_CIRCUIT_3] = strTemp.GetAt(2);

	nIndex = m_ctrlComboOccerInfo.GetCurSel();
	m_eventBuf[SI_EVENT_BUF_OCCUR_INFO] = g_lpszOccurInfo[nIndex];
}

void CDlgEventTest::OnSelchangeComboCommand()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nIndex = 0;
	nIndex = m_ctrlComboCommand.GetCurSel();

	if (nIndex == COMMAND_RECOVER)
	{
		m_ctrlComboReceiverNo.SetCurSel(0);
		m_ctrlComboUnitNo.SetCurSel(0);
		m_ctrlComboSystemNo.SetCurSel(0);
		m_ctrlComboCircuitNo.SetCurSel(0);
		m_ctrlComboOccerInfo.SetCurSel(OCCUR_INFO_OCCUR);
	}
}
