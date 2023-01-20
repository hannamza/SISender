
// PMDlg.h : 헤더 파일
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"

#define		LIST_MESSAGE			(WM_USER+1004)

typedef struct  
{
	bool use;
	int SIType;
	int processId;
	TCHAR processName[MAX_PATH];
}PROCESS_INFO;

#define TIMER_PROCESS_CHECK_ID			200
#define TIMER_PROCESS_CHECK_PERIOD		5000

// CPMDlg 대화 상자
class CPMDlg : public CDialogEx
{
// 생성입니다.
public:
	CPMDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PM_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


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
	bool m_bSMCheck;
	CListCtrl m_ctrlListProcess;
	std::vector<PROCESS_INFO*> m_vecProcessInfo;
	CProcWrapper m_pw;
	ULONGLONG m_nPreAliveCount[PROCESS_TOTAL_COUNT];

	CRichEditCtrl m_richLog;
	
	BOOL InitProcessInfo();
	void KillAllExe();
	void ReleaseProcessInfo();
	void InitControl();
	BOOL ExcuteAllExe();
	void QuitGracefullyAllExe();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnLogMessage(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnListMessage(WPARAM wParam, LPARAM lParam);
	CButton m_checkScroll;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
