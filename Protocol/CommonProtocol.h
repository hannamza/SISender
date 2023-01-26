#pragma once

#include "EncDec.h"

//#include "KocomProtocol.h"

#pragma pack(push, 1)

class ProtocolHeader
{
public:
	ProtocolHeader() { protocol = 0; size = 0; };
	struct
	{
		unsigned short protocol;// : 16;
		int size;// unsigned size : 32;
	};

	enum ProtocolCommand
	{
		Keep_Alive = 1,

		RequestLogin,			// login(manager)
		ResponseLogin,

		// mobile protocol ---------------------------------------- order 및 sequence 변경 되면 안됨(mobile 연동)

		RequestSetToken,		// 4. 토큰, 전화번호 전송
		ResponseSetToken,

		RequestGetEventList,	// 일련번호 이후의 이벤트 리스트 요청
		ResponseGetEventList,

		// manager protocol ---------------------------------------- 이하로 변경 작업을 통하여 가능

		RequestAdminLogin,		// 관리자 로그인(GFS)
		ResponseAdminLogin,

		RequestAddNewSystem,	// 10. 건물, 단지 등 시스템 추가
		ResponseAddNewSystem,

		RequestModSystem,		// 수정
		ResponseModSystem,

		RequestDelSystem,		// 삭제
		ResponseDelSystem,

		RequestGetSystemList,	// 시스템 목록 요청
		ResponseGetSystemList,

		// manager

		RequestGetManagerList,	// 18. 관리자 목록 요청
		ResponseGetManagerList,

		RequestAddManager,		// 관리자 추가
		ResponseAddManager,

		RequestModManager,		// 관리자 수정
		ResponseModManager,

		RequestDelManager,		// 관리자 삭제
		ResponseDelManager,

		// user

		RequestGetUserList,		// 26. 사용자 목록 요청
		ResponseGetUserList,

		RequestGetUserTokenList, // 사용자 토큰 목록 요청
		ResponseGetUserTokenList,

		RequestAddUser,			// 사용자 추가
		ResponseAddUser,

		RequestModUser,			// 사용자 수정
		ResponseModUser,

		RequestDelUser,			// 사용자 삭제
		ResponseDelUser,

		// sender protocol --------------------------------------------

		RequestAddEvent,		// 36. 이벤트 발생
		ResponseAddEvent,

		RequestManagerInfo,		// 관리자 로그인 및 관리자 정보
		ResponseManagerInfo,

		DefineEndProtocol
	};
};

//------------- COMMON ---------------------------------------------------

// common request
class ProtocolRequest : public ProtocolHeader
{
public:
	ProtocolRequest()
	{
		memset(this, 0, sizeof(*this));
		protocol = 0;
		size = sizeof(*this);
	}
	int nValue;
};

// common response
class ProtocolResponse : public ProtocolHeader
{
public:
	ProtocolResponse()
	{
		memset(this, 0, sizeof(*this));
		protocol = 0;
		size = sizeof(*this);
	}
	int nResult;
};

//------------------------------------------------------------------------

// login(manager) - ResponseLogin(1: success, 0: fail)
class ProtocolRequestLogin : public ProtocolHeader
{
public:
	ProtocolRequestLogin()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestLogin;
		size = sizeof(*this);
	}
	CHAR szID[32];
	CHAR szPW[32];
};

// mobile protocol ----------------------------------------

// 토큰, 전화번호 전송
// response - ResponseSetMasterKey(1: success, 0: fail)
class ProtocolRequestSetToken : public ProtocolHeader
{
public:
	ProtocolRequestSetToken()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestSetToken;
		size = sizeof(*this);
	}
	CHAR szPhoneNo[16];
	CHAR szToken[256];
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

//--------------------------------------------------------------------------------------
// 일련번호 이후의 이벤트 리스트 요청
class ProtocolRequestGetEventList : public ProtocolHeader
{
public:
	ProtocolRequestGetEventList()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestGetEventList;
		size = sizeof(*this);
	}
	int nType;
	int nSequenceNo;
	char szPhoneNo[16];
};
class ProtocolResponseGetEventList : public ProtocolHeader
{
public:
	ProtocolResponseGetEventList()
	{
		memset(this, 0, sizeof(*this));
		protocol = ResponseGetEventList;
		size = 0;
	}
	int nType;
	int nCount;
	char szEventList[0]; // event1;event2;event3
};
//--------------------------------------------------------------------------------------
// 건물, 단지 등 시스템 추가
// response - ResponseAddNewSystem (1: success, 0: fail)
class ProtocolRequestAddNewSystem : public ProtocolHeader
{
public:
	ProtocolRequestAddNewSystem()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestAddNewSystem;
		size = sizeof(*this);
	}
	CHAR szSystemName[64];
	CHAR szLocation[256];
	CHAR szEtc[512];
};
//--------------------------------------------------------------------------------------
// 수정
// response - ResponseModSystem(1: success, 0: fail, 2: 이미 폰이 등록되어 있음)
class ProtocolRequestModSystem : public ProtocolHeader
{
public:
	ProtocolRequestModSystem()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestModSystem;
		size = sizeof(*this);
	}
	int nSequenceNo;
	CHAR szSystemName[64];
	CHAR szLocation[256];
	CHAR szEtc[512];
};
//--------------------------------------------------------------------------------------
// 삭제
// response - ResponseDelSystem(1: success, 0: fail)
class ProtocolRequestDelSystem : public ProtocolHeader
{
public:
	ProtocolRequestDelSystem()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestDelSystem;
		size = sizeof(*this);
	}
	int nSequenceNo;
};
//--------------------------------------------------------------------------------------
// request - RequestGetSystemList
// 시스템 목록 요청
class ProtocolResponseGetSystemList : public ProtocolHeader
{
public:
	ProtocolResponseGetSystemList()
	{
		memset(this, 0, sizeof(*this));
		protocol = ResponseGetSystemList;
		size = sizeof(*this);
	}
	int nCount;
	CHAR szSystemInfo[0];
};

// manager
//--------------------------------------------------------------------------------------
//request - RequestGetManagerList (nSequenceNo - work site seq)
class ProtocolResponseGetManagerList : public ProtocolHeader
{
public:
	ProtocolResponseGetManagerList()
	{
		memset(this, 0, sizeof(*this));
		protocol = ResponseGetManagerList;
		size = sizeof(*this);
	}
	struct managerInfo
	{
		int nSeq;
		int nWorksiteSeq;
		int nUserLimit;
		char szID[32];
		char szEtc[256];
		char szRegdate[32];
	};
	int nCount;
	managerInfo info[0];
};
//--------------------------------------------------------------------------------------
//response - ResponseAddManager
class ProtocolRequestAddManager : public ProtocolHeader
{
public:
	ProtocolRequestAddManager()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestAddManager;
		size = sizeof(*this);
	}
	int nWorksiteSeq;
	int nUserLimit;
	char szID[32];
	char szPW[32];
	char szEtc[256];
};

//--------------------------------------------------------------------------------------
//response - ResponseModManager
class ProtocolRequestModManager : public ProtocolHeader
{
public:
	ProtocolRequestModManager()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestModManager;
		size = sizeof(*this);
	}
	int nSeq;
	int nUserLimit;
	char szID[32];
	char szPW[32];
	char szEtc[256];
};

//--------------------------------------------------------------------------------------
//response - ResponseDelManager
class ProtocolRequestDelManager : public ProtocolHeader
{
public:
	ProtocolRequestDelManager()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestDelManager;
		size = sizeof(*this);
	}
	int nSeq;
};

// user
//--------------------------------------------------------------------------------------
//request - RequestGetUserList (nSequenceNo)
class ProtocolResponseGetUserList : public ProtocolHeader
{
public:
	ProtocolResponseGetUserList()
	{
		memset(this, 0, sizeof(*this));
		protocol = ResponseGetUserList;
		size = sizeof(*this);
	}
	struct userInfo
	{
		int nSeq;
		char szName[32];
		char szMobile[32];
		char szEtc[256];
	};
	int nCount;
	userInfo info[0];
};

//--------------------------------------------------------------------------------------
//request - RequestGetUserTokenList (nSequenceNo)
class ProtocolResponseGetUserTokenList : public ProtocolHeader
{
public:
	ProtocolResponseGetUserTokenList()
	{
		memset(this, 0, sizeof(*this));
		protocol = ResponseGetUserTokenList;
		size = sizeof(*this);
	}
	struct userInfo
	{
		CHAR szToken[256];
		CHAR szPhoneNo[16];
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
	int nCount;
	userInfo info[0];
};

//--------------------------------------------------------------------------------------
//response  - ResponseAddUser
class ProtocolRequestAddUser : public ProtocolHeader
{
public:
	ProtocolRequestAddUser()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestAddUser;
		size = sizeof(*this);
	}
	int nManagerSeq;
	char szName[32];
	char szMobile[32];
	char szEtc[256];
};
//--------------------------------------------------------------------------------------
//response  - ResponseModUser
class ProtocolRequestModUser : public ProtocolHeader
{
public:
	ProtocolRequestModUser()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestModUser;
		size = sizeof(*this);
	}
	int nSeq;
	char szName[32];
	char szMobile[32];
	char szEtc[256];
};
//--------------------------------------------------------------------------------------
//response  - ResponseDelUser
class ProtocolRequestDelUser : public ProtocolHeader
{
public:
	ProtocolRequestDelUser()
	{
		memset(this, 0, sizeof(*this));
		protocol = ResponseDelUser;
		size = sizeof(*this);
	}
	int nSeq;
};

//--------------------------------------------------------------------------------------
// 이벤트 발생
class ProtocolRequestAddEvent : public ProtocolHeader
{
public:
	ProtocolRequestAddEvent()
	{
		memset(this, 0, sizeof(*this));
		protocol = RequestAddEvent;
		size = sizeof(*this);
	}
	INT idx;
	CHAR szID[32];
	CHAR szEvent[100 * 1024];
};
//response - ResponseAddEvent (nSequenceNo: success, 0: fail)

//--------------------------------------------------------------------------------------
// 관리자 로그인/정보 요청
class ProtocolResponseManagerInfo : public ProtocolHeader
{
public:
	ProtocolResponseManagerInfo()
	{
		memset(this, 0, sizeof(*this));
		protocol = ResponseManagerInfo;
		size = sizeof(*this);
	}
	int nResult;
	int nWorkSiteSeq;
	int nManagerSeq;
	int nLimitUser;
};

#pragma pack(pop, 1)