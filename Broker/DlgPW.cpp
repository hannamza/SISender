// DlgPW.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Broker.h"
#include "DlgPW.h"
#include "afxdialogex.h"


// CDlgPW ��ȭ �����Դϴ�.

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


// CDlgPW �޽��� ó�����Դϴ�.


void CDlgPW::OnOK()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_PW);
	pEdit->GetWindowText(m_strPW);

	CDialogEx::OnOK();
}


void CDlgPW::OnCancel()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	CDialogEx::OnCancel();
}


BOOL CDlgPW::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
