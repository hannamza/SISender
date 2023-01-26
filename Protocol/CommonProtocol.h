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

		// mobile protocol ---------------------------------------- order �� sequence ���� �Ǹ� �ȵ�(mobile ����)

		RequestSetToken,		// 4. ��ū, ��ȭ��ȣ ����
		ResponseSetToken,

		RequestGetEventList,	// �Ϸù�ȣ ������ �̺�Ʈ ����Ʈ ��û
		ResponseGetEventList,

		// manager protocol ---------------------------------------- ���Ϸ� ���� �۾��� ���Ͽ� ����

		RequestAdminLogin,		// ������ �α���(GFS)
		ResponseAdminLogin,

		RequestAddNewSystem,	// 10. �ǹ�, ���� �� �ý��� �߰�
		ResponseAddNewSystem,

		RequestModSystem,		// ����
		ResponseModSystem,

		RequestDelSystem,		// ����
		ResponseDelSystem,

		RequestGetSystemList,	// �ý��� ��� ��û
		ResponseGetSystemList,

		// manager

		RequestGetManagerList,	// 18. ������ ��� ��û
		ResponseGetManagerList,

		RequestAddManager,		// ������ �߰�
		ResponseAddManager,

		RequestModManager,		// ������ ����
		ResponseModManager,

		RequestDelManager,		// ������ ����
		ResponseDelManager,

		// user

		RequestGetUserList,		// 26. ����� ��� ��û
		ResponseGetUserList,

		RequestGetUserTokenList, // ����� ��ū ��� ��û
		ResponseGetUserTokenList,

		RequestAddUser,			// ����� �߰�
		ResponseAddUser,

		RequestModUser,			// ����� ����
		ResponseModUser,

		RequestDelUser,			// ����� ����
		ResponseDelUser,

		// sender protocol --------------------------------------------

		RequestAddEvent,		// 36. �̺�Ʈ �߻�
		ResponseAddEvent,

		RequestManagerInfo,		// ������ �α��� �� ������ ����
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

// ��ū, ��ȭ��ȣ ����
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
// �Ϸù�ȣ ������ �̺�Ʈ ����Ʈ ��û
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
// �ǹ�, ���� �� �ý��� �߰�
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
// ����
// response - ResponseModSystem(1: success, 0: fail, 2: �̹� ���� ��ϵǾ� ����)
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
// ����
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
// �ý��� ��� ��û
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
// �̺�Ʈ �߻�
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
// ������ �α���/���� ��û
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