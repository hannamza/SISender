#pragma once


// CTestDlg ��ȭ �����Դϴ�.

class CTestDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CTestDlg)

public:
	CTestDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CTestDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_TEST };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()


public:
	afx_msg void OnBnClickedRadio(UINT uiID);
	virtual BOOL OnInitDialog();
};
