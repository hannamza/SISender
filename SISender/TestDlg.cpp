// TestDlg.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "SISender.h"
#include "TestDlg.h"
#include "afxdialogex.h"


// CTestDlg 대화 상자입니다.

IMPLEMENT_DYNAMIC(CTestDlg, CDialogEx)

CTestDlg::CTestDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_TEST, pParent)
{

}

CTestDlg::~CTestDlg()
{
}

void CTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CTestDlg, CDialogEx)
	ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_FIRE, IDC_RADIO_RESTORE, &CTestDlg::OnBnClickedRadio)
END_MESSAGE_MAP()


// CTestDlg 메시지 처리기입니다.

BOOL CTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CTestDlg::OnBnClickedRadio(UINT uiID)
{
	UpdateData(TRUE);

	switch (uiID)
	{
	case IDC_RADIO_FIRE:
		AfxMessageBox(_T("화재"));
		break;
	case IDC_RADIO_GAS:
		AfxMessageBox(_T("가스"));
		break;
	case IDC_RADIO_SURVEILLANCE:
		AfxMessageBox(_T("감시"));
		break;
	case IDC_RADIO_TRIP:
		AfxMessageBox(_T("단선"));
		break;
	case IDC_RADIO_RESTORE:
		AfxMessageBox(_T("복구"));
		break;
	}
}