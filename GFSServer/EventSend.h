#pragma once
#include <queue>
#include <thread>
#include <afxtempl.h>
using namespace std;

#define		ID_CNT			5

typedef struct STATE_DATA
{
	char code1;
	char code2;
	char event_code;
	char recv_number[2];
	char unit_number[2];
	char seperate;
	char type_number;
	char line_number[3];
	char event;
}stateData, *pStateData;

struct userInfo
{
	CString szToken;
	CString szPhoneNo;
	int nMobileType;
	int nFire;
	int nGas;
	int nSpy;
	int nLine;
	int nAlert;
	int nHour;
	int nMin;
	int nEndHour;
	int nEndMin;
	int nUseTime;
};

class CEventSend : public TSingleton<CEventSend>
{
public:
	CEventSend();
	~CEventSend();

public:
	bool InitEvent();
	void SendEvent(BYTE* pData);
	void SetMainWnd(CWnd* pWnd);

protected:
	queue<BYTE*> m_sendQueue;
	queue<BYTE*> m_lineQueue;
	queue<BYTE*> m_gasQueue;
	queue<BYTE*> m_spyQueue;
	queue<BYTE*> m_fireQueue;
	queue<BYTE*> m_fixQueue;

	thread* m_pThread;
	CCriticalSectionEx m_sync, mIDSync, m_sendSync;
	//CString m_IDList[100];
	int m_nIDCount;
	CWnd* m_pWnd;
	CList<userInfo*, userInfo*> m_list;

	DWORD m_dwLine, m_dwGas, m_dwSpy, m_dwFire, m_dwFix;


private:
	void StartSendEventThread();
	void StopSendEventThread();
	//void ReleaseListUser();
	//void SendAlarm(BYTE* pData, int nSendCount);
	//void ProcessEventQueue(queue<BYTE*> & queue, DWORD & dwValue, bool bSend=false);
	//bool CheckClassify(BYTE* pData, CString & sUni, CString & sTitle, CString & sName, CString & sDisplay, int nSendCount);
	//void CheckAndSend();
	//void SendAll();

public:
	void SendThreadLoop();
};

