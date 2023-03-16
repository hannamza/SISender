#pragma once

#include <winsock2.h>
#include <vector>

#include "TSingleton.h"

class Client;
class Packet;
class IOEvent;

class Server :  public TSingleton<Server>
{
private:
	// Callback Routine
	static void CALLBACK IoCompletionCallback(PTP_CALLBACK_INSTANCE Instance, PVOID Context, PVOID Overlapped, ULONG IoResult, ULONG_PTR NumberOfBytesTransferred, PTP_IO Io);

	// Worker Thread Functions
	static void CALLBACK WorkerPostAccept(PTP_CALLBACK_INSTANCE /* Instance */, PVOID Context, PTP_WORK /* Work */);

	static void CALLBACK WorkerAddClient(PTP_CALLBACK_INSTANCE /* Instance */, PVOID Context);
	static void CALLBACK WorkerRemoveClient(PTP_CALLBACK_INSTANCE /* Instance */, PVOID Context);
	static void CALLBACK WorkerProcessRecvPacket(PTP_CALLBACK_INSTANCE /* Instance */, PVOID Context);

public:
	Server();
	virtual ~Server();

	bool Create(short port, int maxPostAccept);
	void Destroy();

	size_t GetNumClients();
	long GetNumPostAccepts();

private:
	void PostAccept();
	void PostRecv(Client* client);
	void PostSend(Client* client, Packet* packet);

	void OnAccept(IOEvent* event);
	void OnRecv(IOEvent* event, DWORD dwNumberOfBytesTransfered);
	void OnSend(IOEvent* event, DWORD dwNumberOfBytesTransfered);
	void OnClose(IOEvent* event);

	void AddClient(Client* client);
	void RemoveClient(Client* client);

	void Echo(Packet* packet);
	void Send(Packet* packet, BYTE* pData, int nSize, int nCommand);
	void SendAll(BYTE* pData, int nSize);
	void SendAndSetLoggedInState(Packet* packet, BYTE* pData, int nSize);

protected:

#if 0
	static void GetEventCount(int & nFire, int & nGas, int & nSpy, int & nLine, char* szSeq);


	static void ProcessProtocolRequestLogin(BYTE* pData, Packet* packet);	// login(manager)
	static void ProcessProtocolRequestManagerInfo(BYTE* pData, Packet* packet);
	static void ProcessProtocolRequestSetToken(BYTE* pData, Packet* packet);	// 토큰, 전화번호 전송
	static void ProcessProtocolRequestGetEventList(BYTE* pData, Packet* packet);	// 일련번호 이후의 이벤트 리스트 요청
	static void ProcessProtocolRequestAdminLogin(BYTE* pData, Packet* packet);		// 관리자 로그인
	// system
	static void ProcessProtocolRequestAddNewSystem(BYTE* pData, Packet* packet);	// 건물, 단지 등 시스템 추가
	static void ProcessProtocolRequestModSystem(BYTE* pData, Packet* packet);		// 수정
	static void ProcessProtocolRequestDelSystem(BYTE* pData, Packet* packet);		// 삭제
	static void ProcessProtocolRequestGetSystemList(BYTE* pData, Packet* packet);	// 시스템 목록 요청
	// manager
	static void ProcessProtocolRequestGetManagerList(BYTE* pData, Packet* packet);	// 매니저 목록 요청
	static void ProcessProtocolRequestAddManager(BYTE* pData, Packet* packet);		// 매니저 추가
	static void ProcessProtocolRequestModManager(BYTE* pData, Packet* packet);		// 매니저 수정
	static void ProcessProtocolRequestDelManager(BYTE* pData, Packet* packet);		// 매니저 삭제
	// user
	static void ProcessProtocolRequestGetUserList(BYTE* pData, Packet* packet);		// 해당 시스템에 등록된 유저 목록 요청
	static void ProcessProtocolRequestGetUserTokenList(BYTE* pData, Packet* packet);
	static void ProcessProtocolRequestAddUser(BYTE* pData, Packet* packet);			// 유저 추가
	static void ProcessProtocolRequestModUser(BYTE* pData, Packet* packet);			// 유저 수정
	static void ProcessProtocolRequestDelUser(BYTE* pData, Packet* packet);			// 유저 삭제
	// event
	static void ProcessProtocolRequestAddEvent(BYTE* pData, Packet* packet);		// 이벤트 추가
	static void ProcessProtocolRequestSetToken1(BYTE* pData, Packet* packet);
	static void ProcessProtocolRequestGetEventListEnc(BYTE* pData, Packet* packet);
#endif

	static void ProcessGFSProtocolResponseError(int nResult, Packet* packet);
	static void ProcessGFSProtocolRequestLogin(BYTE* pData, Packet* packet);
	static void ProcessGFSProtocolRequestKeepAlive(BYTE* pData, Packet* packet);

public:
	void CheckKeepAliveGFSClients();
	void ProcessGFSProtocolUnSolicitedEvent(BYTE* pData);

private:
	Server& operator=(Server& rhs);
	Server(const Server& rhs);

private:
	TP_IO* m_pTPIO;
	SOCKET m_listenSocket;

	TP_WORK* m_AcceptTPWORK; 

	typedef std::vector<Client*> ClientList;
	ClientList m_Clients;

	int	m_MaxPostAccept;
	volatile long m_NumPostAccept;

	CRITICAL_SECTION m_CSForClients;

	TP_CALLBACK_ENVIRON m_ClientTPENV;
	TP_CLEANUP_GROUP* m_ClientTPCLEAN;

	volatile bool m_ShuttingDown;
};
