#pragma once
#include "afxwin.h"


// CDlgEventTest ��ȭ �����Դϴ�.

class CDlgEventTest : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgEventTest)

public:
	CDlgEventTest(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CDlgEventTest();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TEST_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

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
