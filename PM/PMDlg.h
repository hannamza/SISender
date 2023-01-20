
// PMDlg.h : ��� ����
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

// CPMDlg ��ȭ ����
class CPMDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CPMDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PM_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


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
