
// GFSClientDlg.h : 헤더 파일
//

#pragma once
#include "EventSend.h"
#include "afxwin.h"
#include "afxcmn.h"

// CGFSClientDlg 대화 상자
class CGFSClientDlg : public CDialogEx
{
// 생성입니다.
public:
	CGFSClientDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GFSCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

private:
	BOOL m_bStarted;
	BOOL m_bKilled;

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
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
