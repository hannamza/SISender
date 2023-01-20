#pragma once

#pragma pack(push, 1)

// 1. 코콤

enum {
	BIND,
	BIND_ACK,
	ALIVE,
	ALIVE_ACK,
	FIRE_ALARM,
	FIRE_ALARM_ACK,
	ERROR_ACK
}KOCOM_ENUM;

#define KOCOM_KEEP_ALIVE_PERIOD	50	// 50초

#define KOCOM_NONE_TYPE 0x00000000
#define KOCOM_FIRE_TYPE 0x36000000

#define KOCOM_MSG_BIND				0
#define KOCOM_MSG_BIND_ACK			1
#define KOCOM_MSG_ALIVE				4
#define KOCOM_MSG_ALIVE_ACK			5
#define KOCOM_MSG_FIRE_ALARM		800
#define KOCOM_MSG_FIRE_ALARM_ACK	801
#define KOCOM_MSG_ERROR_ACK			10000

#define KOCOM_SUCCESS	0

#define KOCOM_ERROR_AUTH_FAIL_ID		1
#define KOCOM_ERROR_AUTH_FAIL_PASS		2
#define	KOCOM_ERROR_OFF_HOME			3
#define	KOCOM_ERROR_RE_TRY_DATA			7
#define	KOCOM_ERROR_OVERFLOW_Q_SIZE		1100

#define KOCOM_HEADER_KEY_VALUE		0x12345678

#define KOCOM_FIRE_ALARM_FIRE_OCCURED	1
#define KOCOM_FIRE_ALARM_FIRE_CLEAR		2
#define KOCOM_FIRE_ALARM_ALL_CLEAR		3

struct KOCOMProtocolHeader
{
	int headerKey;
	int msgType;
	int msgLength;
	int town;
	int dong;
	int ho;
	int reserved;
};

class KOCOMProtocolBind
{
public:
	KOCOMProtocolBind()
	{
		memset(this, 0, sizeof(*this));
	}

	KOCOMProtocolHeader header;
	int homeVersion;
	int nKind;
	int nVersion[4];
	CHAR szId[40];			// utf-8
	CHAR szPass[40];		// utf-8
};

class KOCOMProtocolBindAck
{
public:
	KOCOMProtocolBindAck()
	{
		memset(this, 0, sizeof(*this));
	}

	KOCOMProtocolHeader header;
	int nResult;
};

class KOCOMProtocolAlive
{
public:
	KOCOMProtocolAlive()
	{
		memset(this, 0, sizeof(*this));
	}

	KOCOMProtocolHeader header;
	int homeVersion;
	int nKind;
	int nVersion[4];
};

class KOCOMProtocolAliveAck
{
public:
	KOCOMProtocolAliveAck()
	{
		memset(this, 0, sizeof(*this));
	}

	KOCOMProtocolHeader header;
	int nResult;
};

class KOCOMProtocolFireAlarm
{
public:
	KOCOMProtocolFireAlarm()
	{
		memset(this, 0, sizeof(*this));
	}

	KOCOMProtocolHeader header;
	int nAlarm;
	int nFDong;
	int nFHo;
	int nFFloor;
	CHAR szFMsg[80];	//한글은 utf-8
	int nReserved;
};

class KOCOMProtocolFirAlarmAck
{
public:
	KOCOMProtocolFirAlarmAck()
	{
		memset(this, 0, sizeof(*this));
	}

	KOCOMProtocolHeader header;
	int nResult;
	int nReserved;
};

class KOCOMProtocolErrorAck
{
public:
	KOCOMProtocolErrorAck()
	{
		memset(this, 0, sizeof(*this));
	}

	KOCOMProtocolHeader header;
	int nResult;
};

#pragma pack(pop, 1)