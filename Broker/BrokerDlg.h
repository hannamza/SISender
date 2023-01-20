
// BrokerDlg.h : ��� ����
//

#pragma once
#include "EventSend.h"
#include "afxwin.h"
#include "afxcmn.h"

// CBrokerDlg ��ȭ ����
class CBrokerDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CBrokerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_BROKER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

private:
	bool m_bStarted;
	bool m_bKilled;

private:
	void RegistryTrayIcon();

// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString m_strLogFolderName;
	CButton m_checkScroll;
	CComboBox m_comboPort;
	CRichEditCtrl m_richLog;
	ULONGLONG m_nAliveCount;

	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnTrayNotification(WPARAM, LPARAM);
	afx_msg void OnMenuStart();
	afx_msg void OnMenuStop();
	afx_msg void OnMenuPopup();
	afx_msg void OnMenuExit();
	afx_msg LRESULT OnLogMessage(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonTest();
};
