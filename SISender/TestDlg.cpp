// TestDlg.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "SISender.h"
#include "TestDlg.h"
#include "afxdialogex.h"


// CTestDlg ��ȭ �����Դϴ�.

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


// CTestDlg �޽��� ó�����Դϴ�.

BOOL CTestDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.

	return TRUE;  // return TRUE unless you set the focus to a control
				  // ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void CTestDlg::OnBnClickedRadio(UINT uiID)
{
	UpdateData(TRUE);

	switch (uiID)
	{
	case IDC_RADIO_FIRE:
		AfxMessageBox(_T("ȭ��"));
		break;
	case IDC_RADIO_GAS:
		AfxMessageBox(_T("����"));
		break;
	case IDC_RADIO_SURVEILLANCE:
		AfxMessageBox(_T("����"));
		break;
	case IDC_RADIO_TRIP:
		AfxMessageBox(_T("�ܼ�"));
		break;
	case IDC_RADIO_RESTORE:
		AfxMessageBox(_T("����"));
		break;
	}
}