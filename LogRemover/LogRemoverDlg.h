
// LogRemoverDlg.h : 헤더 파일
//

#pragma once

#include <thread>
#include "afxcmn.h"
using namespace std;

// CLogRemoverDlg 대화 상자
class CLogRemoverDlg : public CDialogEx
{
// 생성입니다.
public:
	CLogRemoverDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_LOGREMOVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

protected:
	CString m_sPath;
	thread* m_Thread;

public:
	void DeleteLogfiles(CString sPath, CTime & tDel, int nYear, int nMon, int nType);

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	void RegistryTrayIcon();

public:
	CListCtrl m_ctrlListLogFolder;
	std::vector<CString> m_vecLogFolder;
	ULONGLONG m_nAliveCount;
	bool m_bStarted;

	void GetLogFolderList();
	void InitControl();

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnDestroy();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonStop();
	afx_msg LRESULT OnTrayNotification(WPARAM, LPARAM);
	afx_msg void OnMenuStart();
	afx_msg void OnMenuStop();
	afx_msg void OnMenuPopup();
	afx_msg void OnMenuExit();
	afx_msg void OnClose();
};
