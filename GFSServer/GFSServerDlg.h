
// GFSServerDlg.h : ��� ����
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "EventSend.h"

// CGFSServerDlg ��ȭ ����
class CGFSServerDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CGFSServerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GFSSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

private:
	int m_nServerPort;
	int m_nLimitClient;
	BOOL m_bStartServer;

public:
	BOOL m_bSMCheck;
	ULONGLONG m_nAliveCount;
	long m_nSMIndex;

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
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnLogMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	BOOL GetServerInfoFromIniFile();
	BOOL CheckSMTimeChanged(SYSTEMTIME preTime, SYSTEMTIME curTime);
	BOOL CheckSMIndexChanged();	//���� �����޸� ���� ������ �̺�Ʈ �ε����� üũ�Ѵ�.
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTrayNotification(WPARAM, LPARAM);
	afx_msg void OnMenuStart();
	afx_msg void OnMenuStop();
	afx_msg void OnMenuPopup();
	afx_msg void OnMenuExit();
	void RegistryTrayIcon();
	afx_msg void OnClose();
};
