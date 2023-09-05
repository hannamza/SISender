
// GFSServerDlg.h : 헤더 파일
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "EventSend.h"

// CGFSServerDlg 대화 상자
class CGFSServerDlg : public CDialogEx
{
// 생성입니다.
public:
	CGFSServerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_GFSSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

private:
	int m_nServerPort;
	int m_nLimitClient;
	BOOL m_bStartServer;

public:
	BOOL m_bSMCheck;
	ULONGLONG m_nAliveCount;
	long m_nSMIndex;

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
	CRichEditCtrl m_richLog;
	CButton m_checkScroll;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnLogMessage(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	BOOL GetServerInfoFromIniFile();
	BOOL CheckSMTimeChanged(SYSTEMTIME preTime, SYSTEMTIME curTime);
	BOOL CheckSMIndexChanged();	//현재 공유메모리 상의 마지막 이벤트 인덱스를 체크한다.
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTrayNotification(WPARAM, LPARAM);
	afx_msg void OnMenuStart();
	afx_msg void OnMenuStop();
	afx_msg void OnMenuPopup();
	afx_msg void OnMenuExit();
	void RegistryTrayIcon();
	afx_msg void OnClose();
};
