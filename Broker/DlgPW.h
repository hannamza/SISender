#pragma once


// CDlgPW 대화 상자입니다.

class CDlgPW : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgPW)

public:
	CDlgPW(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgPW();

public:
	CString m_strPW;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PW_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();
	virtual void OnCancel();
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
