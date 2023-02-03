#pragma once

//외부 업체 연계 ID, 0: 테스트(기존 GFSM Server), 1: Kocom
enum {
	SI_TEST,
	KOCOM,
	SI_TOTAL_COUNT
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

//수신기 event 명령 코드 enum
enum {
	COMMAND_FIRE,
	COMMAND_GAS,
	COMMAND_SURVEIL,
	COMMAND_CUT,
	COMMAND_RECOVER,
	COMMAND_TOTAL_COUNT
};

// 발생 정보 enum
enum {
	OCCUR_INFO_OCCUR,
	OCCUR_INFO_RECOVER,
	OCCUR_INFO_TOTAL_COUNT
};

//수신기 event 명령 코드 문자열
static const TCHAR* g_lpszCommandString[] = {
	_T("화재"),	//F
	_T("가스"),	//G
	_T("감시"),	//S
	_T("단선"),	//T
	_T("복구"),	//R
	NULL
};

//수신기 event 명령 코드
static const TCHAR g_lpszCommand[] = {
	_T('F'),	//화재
	_T('G'),	//가스
	_T('S'),	//감시
	_T('T'),	//단선
	_T('R'),	//복구
	NULL
};

//수신기 event 발생 문자열
static const TCHAR* g_lpszOccurInfoString[] = {
	_T("발생"),
	_T("복구"),
	NULL
};

//수신기 event 발생 코드
static const TCHAR g_lpszOccurInfo[] = {
	_T('N'),
	_T('F'),
	NULL
};

//수신기 최대 개수
#define RECEIVER_MAX_COUNT	32

//유닛 최대 개수
#define UNIT_MAX_COUNT	64

//계통 최대 개수
#define SYSTEM_MAX_COUNT	4

//회로 최대 개수
#define CIRCUIT_MAX_COUNT	253


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
#define	WM_TRAY_NOTIFICATION	(WM_USER+1003)

#define BUFF_SIZE 4096	//DeviceInfo.h에서 옮김

// Test Flag - 디버깅 시 사용하기 위한 플래그로 세팅되면 
// Broker와 SISender가 SM processRun에 따라 종료하지 않고
// SISender의 실행 시 인덱스를 PM으로부터 받지 않고 2로 고정
//#define TEST_MODE_FLAG
