#pragma once

//�ܺ� ��ü ���� ID, 0: �׽�Ʈ(���� GFSM Server), 1: Kocom
enum {
	SI_TEST,
	KOCOM,
};

//SI Sender ���μ��� 
enum {
	BROKER,
	SI_SENDER1,
	SI_SENDER2,
	SI_SENDER3,
	LOG_REMOVER,
	PROCESS_TOTAL_COUNT
};

//���ű� event ���� ����
enum {
	SI_EVENT_BUF_COMPANY_1,
	SI_EVENT_BUF_COMPANY_2,
	SI_EVENT_BUF_COMMAND,
	SI_EVENT_BUF_FIRE_RECEIVER_1,
	SI_EVENT_BUF_FIRE_RECEIVER_2,
	SI_EVENT_BUF_UNIT_1,
	SI_EVENT_BUF_UNIT_2,
	SI_EVENT_BUF_DASH,
	SI_EVENT_BUF_SYSTEM,
	SI_EVENT_BUF_CIRCUIT_1,
	SI_EVENT_BUF_CIRCUIT_2,
	SI_EVENT_BUF_CIRCUIT_3,
	SI_EVENT_BUF_OCCUR_INFO,
	SI_EVENT_BUF_EOP_1,
	SI_EVENT_BUF_EOP_2,
	SI_EVENT_BUF_SIZE
};

//���μ��� ��
static const TCHAR* g_lpszProcessName[] = {
	_T("Broker.exe"),
	_T("SISender.exe"),
	_T("SISender.exe"),
	_T("SISender.exe"),
	_T("LogRemover.exe"),
	NULL
};

//��ü ��

static const TCHAR* g_lpszSIName[] = {
	_T("TEST"),
	_T("KOCOM"),
	NULL
};

// ���μ��� Alive Count Write Ÿ�̸� ID
#define TIMER_ALIVE_COUNT_ID	100

// ���μ��� Alive Count Write Ÿ�̸� �ֱ� (ms)
#define TIMER_ALIVE_COUNT_PERIOD	500

// PM ���� �÷��� Ȯ�� Ÿ�̸� ID
#define TIMER_PM_QUIT_CHECK_ID	300

// PM ���� �÷��� Ȯ�� Ÿ�̸� �ֱ� (ms)
#define TIMER_PM_QUIT_CHECK_PERIOD	1000

// ���α׷� â�� Ʈ���̷� ���� �� ���Ǵ� ����� �޼��� 
#define		WM_TRAY_NOTIFICATION		(WM_USER+1003)

// Test Flag - ����� �� ����ϱ� ���� �÷��׷� ���õǸ� 
// Broker�� SISender�� SM processRun�� ���� �������� �ʰ�
// SISender�� ���� �� �ε����� PM���κ��� ���� �����Ƿ� 1�� ����
//#define TEST_MODE_FLAG
