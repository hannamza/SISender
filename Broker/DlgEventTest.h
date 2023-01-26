#pragma once
#include "afxwin.h"


// CDlgEventTest 대화 상자입니다.

class CDlgEventTest : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgEventTest)

public:
	CDlgEventTest(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgEventTest();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEST_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonConfirm();
	afx_msg void OnBnClickedButtonCancel();
	CComboBox m_ctrlComboCommand;
	CComboBox m_ctrlComboReceiverNo;
	CComboBox m_ctrlComboUnitNo;
	CComboBox m_ctrlComboSystemNo;
	CComboBox m_ctrlComboCircuitNo;
	CComboBox m_ctrlComboOccerInfo;

	BYTE m_eventBuf[SI_EVENT_BUF_SIZE];
	void InitControl();
	void MakeEventBuf();
	afx_msg void OnSelchangeComboCommand();
};
