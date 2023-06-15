// DlgPW.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Broker.h"
#include "DlgPW.h"
#include "afxdialogex.h"


// CDlgPW 대화 상자입니다.

IMPLEMENT_DYNAMIC(CDlgPW, CDialogEx)

CDlgPW::CDlgPW(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_PW_DIALOG, pParent)
{
	m_strPW = _T("");
}

CDlgPW::~CDlgPW()
{
}

void CDlgPW::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgPW, CDialogEx)
END_MESSAGE_MAP()


// CDlgPW 메시지 처리기입니다.


void CDlgPW::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PW);
	pEdit->GetWindowText(m_strPW);

	CDialogEx::OnOK();
}


void CDlgPW::OnCancel()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	CDialogEx::OnCancel();
}


BOOL CDlgPW::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
