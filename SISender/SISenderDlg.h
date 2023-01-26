
// SISenderDlg.h : ��� ����
//

#pragma once
#include "EventSend.h"
#include "afxcmn.h"
#include "afxwin.h"

// CSISenderDlg ��ȭ ����
class CSISenderDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CSISenderDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SISENDER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

private:
	bool m_bStarted;
	bool m_bInit;
	bool m_bAutoLogin;

public:
	bool m_bKilled;
	bool m_bSMCheck;

	ULONGLONG m_nAliveCount;

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
	
	CRichEditCtrl m_richLog;
	CButton m_checkScroll;
	CButton m_btnTest;

	BOOL ReadIniFile();
	int GetKeepAlivePeriod();
	BOOL CheckSMTimeChanged(SYSTEMTIME preTime, SYSTEMTIME curTime);	// ���� �ð����� ��ȯ�ؼ� ������ Ȯ���ϴ� �ͺ��� ���� ������ �Ǵ��ؼ� �� �ż��带 ����� ��

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg LRESULT OnTrayNotification(WPARAM, LPARAM);
	afx_msg void OnMenuStart();
	afx_msg void OnMenuStop();
	afx_msg void OnMenuPopup();
	afx_msg void OnMenuExit();
	virtual BOOL PreTranslateMessage(MSG* pMsg);	
	afx_msg void OnBnClickedButtonTest();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnLogMessage(WPARAM wParam, LPARAM lParam);	
	afx_msg void OnClose();
};
