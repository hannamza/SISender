
// GFSClientDlg.h : ��� ����
//

#pragma once
#include "EventSend.h"
#include "afxwin.h"
#include "afxcmn.h"

// CGFSClientDlg ��ȭ ����
class CGFSClientDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CGFSClientDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GFSCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.

private:
	BOOL m_bStarted;
	BOOL m_bKilled;

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
	CButton m_checkScroll;

	BOOL ReadIniFile();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnLogMessage(WPARAM wParam, LPARAM lParam);
	CRichEditCtrl m_richLog;
};
