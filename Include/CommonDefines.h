#pragma once

//�ܺ� ��ü ���� ID, 0: �׽�Ʈ(���� GFSM Server), 1: Kocom
enum {
	SI_TEST,
	KOCOM,
	SI_TOTAL_COUNT
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

//���ű� event ��� �ڵ� enum
enum {
	COMMAND_FIRE,
	COMMAND_GAS,
	COMMAND_SURVEIL,
	COMMAND_CUT,
	COMMAND_RECOVER,
	COMMAND_TOTAL_COUNT
};

// �߻� ���� enum
enum {
	OCCUR_INFO_OCCUR,
	OCCUR_INFO_RECOVER,
	OCCUR_INFO_TOTAL_COUNT
};

//���ű� event ��� �ڵ� ���ڿ�
static const TCHAR* g_lpszCommandString[] = {
	_T("ȭ��"),	//F
	_T("����"),	//G
	_T("����"),	//S
	_T("�ܼ�"),	//T
	_T("����"),	//R
	NULL
};

//���ű� event ��� �ڵ�
static const TCHAR g_lpszCommand[] = {
	_T('F'),	//ȭ��
	_T('G'),	//����
	_T('S'),	//����
	_T('T'),	//�ܼ�
	_T('R'),	//����
	NULL
};

//���ű� event �߻� ���ڿ�
static const TCHAR* g_lpszOccurInfoString[] = {
	_T("�߻�"),
	_T("����"),
	NULL
};

//���ű� event �߻� �ڵ�
static const TCHAR g_lpszOccurInfo[] = {
	_T('N'),
	_T('F'),
	NULL
};

//���ű� �ִ� ����
#define RECEIVER_MAX_COUNT	32

//���� �ִ� ����
#define UNIT_MAX_COUNT	64

//���� �ִ� ����
#define SYSTEM_MAX_COUNT	4

//ȸ�� �ִ� ����
#define CIRCUIT_MAX_COUNT	253


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
#define	WM_TRAY_NOTIFICATION	(WM_USER+1003)

#define BUFF_SIZE 4096	//DeviceInfo.h���� �ű�

// Test Flag - ����� �� ����ϱ� ���� �÷��׷� ���õǸ� 
// Broker�� SISender�� SM processRun�� ���� �������� �ʰ�
// SISender�� ���� �� �ε����� PM���κ��� ���� �ʰ� 2�� ����
//#define TEST_MODE_FLAG
