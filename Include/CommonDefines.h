#pragma once

//외부 업체 연계 ID, 0: 테스트(기존 GFSM Server), 1: Kocom
enum {
	SI_TEST,
	KOCOM,
};

//SI Sender 프로세스 
enum {
	BROKER,
	SI_SENDER1,
	SI_SENDER2,
	SI_SENDER3,
	LOG_REMOVER,
	PROCESS_TOTAL_COUNT
};

//수신기 event 버퍼 구성
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

//프로세스 명
static const TCHAR* g_lpszProcessName[] = {
	_T("Broker.exe"),
	_T("SISender.exe"),
	_T("SISender.exe"),
	_T("SISender.exe"),
	_T("LogRemover.exe"),
	NULL
};

//업체 명

static const TCHAR* g_lpszSIName[] = {
	_T("TEST"),
	_T("KOCOM"),
	NULL
};

// 프로세스 Alive Count Write 타이머 ID
#define TIMER_ALIVE_COUNT_ID	100

// 프로세스 Alive Count Write 타이머 주기 (ms)
#define TIMER_ALIVE_COUNT_PERIOD	500

// PM 종료 플래그 확인 타이머 ID
#define TIMER_PM_QUIT_CHECK_ID	300

// PM 종료 플래그 확인 타이머 주기 (ms)
#define TIMER_PM_QUIT_CHECK_PERIOD	1000

// 프로그램 창을 트레이로 보낼 때 사용되는 사용자 메세지 
#define		WM_TRAY_NOTIFICATION		(WM_USER+1003)

// Test Flag - 디버깅 시 사용하기 위한 플래그로 세팅되면 
// Broker와 SISender가 SM processRun에 따라 종료하지 않고
// SISender의 실행 시 인덱스를 PM으로부터 받지 않으므로 1로 고정
//#define TEST_MODE_FLAG
