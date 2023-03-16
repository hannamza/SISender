#pragma once

#pragma pack(push, 1)

class GFSProtocolHeader 
{
public:
	GFSProtocolHeader() { cmd = 0; size  = 0; reserved = 0; };
	struct
	{
		int cmd;
		int size;
		double reserved;
	};

	enum GFSProtocolCommand {
		ERROR_RESPONSE,
		LOGIN_REQUEST,
		LOGIN_RESPONSE,
		KEEP_ALIVE_REQUEST,
		KEEP_ALIVE_RESPONSE,
		UNSOLICITED_EVENT
	};

	enum GFSProtocolResult {
		UNKNOWN_ERROR,
		WRONG_LOGIN_ID,
		WRONG_LOGIN_PW,
		UNKNOWN_COMMAND,
		WRONG_PACKET_SIZE
	};

	enum GFSProtocolEventType {
		FIRE_INFO = 1,
		GAS_INFO,
		SURVEILLANCE_INFO,
		DISCONNECTION_INFO,
		RESTORATION_INFO
	};

	enum GFSProtocolOccurrence {
		OCCURRENCE = 1,
		RESTORATION,
		RESTORATION_ALL
	};
};

class GFSProtocolLoginRequest : public GFSProtocolHeader {
public:
	GFSProtocolLoginRequest() 
	{
		memset(this, 0, sizeof(*this));
		cmd = GFSProtocolCommand::LOGIN_REQUEST;
		size = sizeof(*this);
	}

	char szID[30];	//UTF-8
	char szPW[30];	//UTF-8
};

class GFSProtocolLoginResponse : public GFSProtocolHeader {
public:
	GFSProtocolLoginResponse()
	{
		memset(this, 0, sizeof(*this));
		cmd = GFSProtocolCommand::LOGIN_RESPONSE;
		size = sizeof(*this);
	}
};

class GFSProtocolKeepAliveRequest : public GFSProtocolHeader {
public:
	GFSProtocolKeepAliveRequest()
	{
		memset(this, 0, sizeof(*this));
		cmd = GFSProtocolCommand::KEEP_ALIVE_REQUEST;
		size = sizeof(*this);
	}
};

class GFSProtocolKeepAliveResponse : public GFSProtocolHeader {
public:
	GFSProtocolKeepAliveResponse()
	{
		memset(this, 0, sizeof(*this));
		cmd = GFSProtocolCommand::KEEP_ALIVE_RESPONSE;
		size = sizeof(*this);
	}
};

class GFSProtocolUnsolicitedEvent : public GFSProtocolHeader {
public:
	GFSProtocolUnsolicitedEvent()
	{
		memset(this, 0, sizeof(*this));
		cmd = GFSProtocolCommand::UNSOLICITED_EVENT;
		size = sizeof(*this);
	}

	char building[30];
	char stair[30];
	char floor[30];
	char room[30];
	char circuit[30];
	int eventType;
	int occurrence;
	double reserved;
};

class GFSProtocolError : public GFSProtocolHeader {
public:
	GFSProtocolError()
	{
		memset(this, 0, sizeof(*this));
		cmd = GFSProtocolCommand::ERROR_RESPONSE;
		size = sizeof(*this);
	}

	int result;
};

#pragma pack(pop, 1)