#pragma once


// CDlgPW ��ȭ �����Դϴ�.

class CDlgPW : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPW)

public:
	CDlgPW(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CDlgPW();

public:
	CString m_strPW;

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PW_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
