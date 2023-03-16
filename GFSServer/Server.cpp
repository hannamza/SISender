#include "stdafx.h"
#include "Server.h"
#include "Client.h"
#include "Packet.h"
#include "IOEvent.h"

#include "Log.h"
#include "Network.h"
#include <iostream>
#include <cassert>
#include <algorithm>

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//---------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------//
/* static */ void CALLBACK Server::IoCompletionCallback(PTP_CALLBACK_INSTANCE /* Instance */, PVOID /* Context */,
														PVOID Overlapped, ULONG IoResult, ULONG_PTR NumberOfBytesTransferred, PTP_IO /* Io */)
{
	IOEvent* event = CONTAINING_RECORD(Overlapped, IOEvent, GetOverlapped());
	assert(event);

	if(IoResult != ERROR_SUCCESS)
	{
		ERROR_CODE(IoResult, "I/O operation failed. type[%d]", event->GetType());

		switch(event->GetType())
		{
		case IOEvent::SEND:
			Server::Instance()->OnSend(event, NumberOfBytesTransferred);
			break;
		}

		Server::Instance()->OnClose(event);
	}
	else
	{	
		switch(event->GetType())
		{
		case IOEvent::ACCEPT:	
			Server::Instance()->OnAccept(event);
			break;

		case IOEvent::RECV:		
			if(NumberOfBytesTransferred > 0)
			{
				Server::Instance()->OnRecv(event, NumberOfBytesTransferred);
			}
			else
			{
				Server::Instance()->OnClose(event);
			}
			break;

		case IOEvent::SEND:
			Server::Instance()->OnSend(event, NumberOfBytesTransferred);
			break;

		default: assert(false); break;
		}
	}

	IOEvent::Destroy(event);
}


void CALLBACK Server::WorkerPostAccept(PTP_CALLBACK_INSTANCE /* Instance */, PVOID Context, PTP_WORK /* Work */)
{
	Server* server = static_cast<Server*>(Context);
	assert(server);

	while(!server->m_ShuttingDown)
	{
		server->PostAccept();
	}
}


void CALLBACK Server::WorkerAddClient(PTP_CALLBACK_INSTANCE /* Instance */, PVOID Context)
{
	Client* client = static_cast<Client*>(Context);
	assert(client);

	Server::Instance()->AddClient(client);
}


void CALLBACK Server::WorkerRemoveClient(PTP_CALLBACK_INSTANCE /* Instance */, PVOID Context)
{
	Client* client = static_cast<Client*>(Context);
	assert(client);

	Server::Instance()->RemoveClient(client);
}


//---------------------------------------------------------------------------------//
//---------------------------------------------------------------------------------//
Server::Server(void)
: m_pTPIO(NULL),
  m_AcceptTPWORK(NULL),
  m_listenSocket(INVALID_SOCKET),
  m_MaxPostAccept(0),
  m_NumPostAccept(0),
  m_ClientTPCLEAN(NULL),
  m_ShuttingDown(true)
{
}


Server::~Server(void)
{
	Destroy();
}


bool Server::Create(short port, int maxPostAccept)
{	
	assert(maxPostAccept > 0);

	m_MaxPostAccept = maxPostAccept;

	// Create critical sections for m_Clients
	InitializeCriticalSection(&m_CSForClients);

	// Create Client Work Thread Env for using cleaning group. We need this for shutting down properly.
	InitializeThreadpoolEnvironment(&m_ClientTPENV);
	m_ClientTPCLEAN = CreateThreadpoolCleanupGroup();
	if (m_ClientTPCLEAN == NULL)
	{
		ERROR_CODE(GetLastError(), "Could not create client cleaning group.");
		return false;
	}
	SetThreadpoolCallbackCleanupGroup(&m_ClientTPENV, m_ClientTPCLEAN, NULL);	

	// Create Listen Socket
	m_listenSocket = Network::CreateSocket(true, port);
	if(m_listenSocket == INVALID_SOCKET)
	{
		return false;
	}

	// Make the address re-usable to re-run the same server instantly.
	bool reuseAddr = true;
	if(setsockopt(m_listenSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuseAddr), sizeof(reuseAddr)) == SOCKET_ERROR)
	{
		ERROR_CODE(WSAGetLastError(), "setsockopt() failed with SO_REUSEADDR.");
		Destroy();
		return false;
	}

	// Create & Start ThreaddPool for socket IO
	m_pTPIO = CreateThreadpoolIo(reinterpret_cast<HANDLE>(m_listenSocket), Server::IoCompletionCallback, NULL, NULL);
	if( m_pTPIO == NULL )
	{
		ERROR_CODE(WSAGetLastError(), "Could not assign the listen socket to the IOCP handle.");
		Destroy();
		return false;
	}

	// Start listening
	StartThreadpoolIo( m_pTPIO );
	if(listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		ERROR_CODE(WSAGetLastError(), "listen() failed.");
		return false;
	}

	// Create Accept worker
	m_AcceptTPWORK = CreateThreadpoolWork(Server::WorkerPostAccept, this, NULL);
	if(m_AcceptTPWORK == NULL)
	{
		ERROR_CODE(GetLastError(), "Could not create AcceptEx worker TPIO.");
		Destroy();
		return false;
	}

	m_ShuttingDown = false;	

	SubmitThreadpoolWork(m_AcceptTPWORK);

	return true;
}


void Server::Destroy()
{
	m_ShuttingDown = true;

	if( m_AcceptTPWORK != NULL )
	{
		WaitForThreadpoolWorkCallbacks( m_AcceptTPWORK, true );
		CloseThreadpoolWork( m_AcceptTPWORK );
		m_AcceptTPWORK = NULL;
	}

	if( m_listenSocket != INVALID_SOCKET )
	{
		Network::CloseSocket(m_listenSocket);
		CancelIoEx(reinterpret_cast<HANDLE>(m_listenSocket), NULL);
		m_listenSocket = INVALID_SOCKET;
	}

	if( m_pTPIO != NULL )
	{
		WaitForThreadpoolIoCallbacks( m_pTPIO, true );
		CloseThreadpoolIo( m_pTPIO );
		m_pTPIO = NULL;
	}

	if (m_ClientTPCLEAN != NULL)
	{
		CloseThreadpoolCleanupGroupMembers(m_ClientTPCLEAN, false, NULL);
		CloseThreadpoolCleanupGroup(m_ClientTPCLEAN);
		DestroyThreadpoolEnvironment(&m_ClientTPENV);
		m_ClientTPCLEAN = NULL;
	}
	
	EnterCriticalSection(&m_CSForClients);
	for(ClientList::iterator itor = m_Clients.begin() ; itor != m_Clients.end() ; ++itor)	
	{
		Client::Destroy(*itor);
	}
	m_Clients.clear();
	LeaveCriticalSection(&m_CSForClients);

	Client::Destroy();
	Packet::Destroy();
	IOEvent::Destroy();

	DeleteCriticalSection(&m_CSForClients);
}


void Server::PostAccept()
{
	// If the number of clients is too big, we can just stop posting aceept.
	// That's one of the benefits from AcceptEx.
	int count = m_MaxPostAccept - m_NumPostAccept;
	if( count > 0 )
	{
		int i = 0;
		for(  ; i < count ; ++i )
		{
			Client* client = Client::Create();
			if( !client )
			{
				break;
			}

			IOEvent* event = IOEvent::Create(IOEvent::ACCEPT, client);
			assert(event);

			StartThreadpoolIo( m_pTPIO );
			if ( FALSE == Network::AcceptEx(m_listenSocket, client->GetSocket(), &event->GetOverlapped()))
			{
				int error = WSAGetLastError();

				if(error != ERROR_IO_PENDING)
				{
					CancelThreadpoolIo( m_pTPIO );

					ERROR_CODE(error, "AcceptEx() failed.");
					Client::Destroy(client);
					IOEvent::Destroy(event);
					break;
				}
			}
			else
			{
				OnAccept(event);
				IOEvent::Destroy(event);
			}
		}

		InterlockedExchangeAdd(&m_NumPostAccept, i);	

		//trace("[%d] Post AcceptEx : %d", GetCurrentThreadId(), m_NumPostAccept);
	}
	else
	{
		Sleep(1);
	}
}


void Server::PostRecv(Client* client)
{
	assert(client);

	WSABUF recvBufferDescriptor;
	recvBufferDescriptor.buf = reinterpret_cast<char*>(client->GetRecvBuff());
	recvBufferDescriptor.len = Client::MAX_RECV_BUFFER;

	DWORD numberOfBytes = 0;
	DWORD recvFlags = 0;

	IOEvent* event = IOEvent::Create(IOEvent::RECV, client);
	assert(event);

	StartThreadpoolIo(client->GetTPIO());

	if(WSARecv(client->GetSocket(), &recvBufferDescriptor, 1, &numberOfBytes, &recvFlags, &event->GetOverlapped(), NULL) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		if(error != ERROR_IO_PENDING)
		{
			CancelThreadpoolIo(client->GetTPIO());

			ERROR_CODE(error, "WSARecv() failed.");

			OnClose(event);
			IOEvent::Destroy(event);
		}
	}
	else
	{
		// In this case, the completion callback will have already been scheduled to be called.
	}
}


void Server::PostSend(Client* client, Packet* packet)
{
	assert(client);
	assert(packet);

	WSABUF recvBufferDescriptor;
	recvBufferDescriptor.buf = reinterpret_cast<char*>(packet->GetData());
	recvBufferDescriptor.len = packet->GetSize();

	DWORD sendFlags = 0;

	IOEvent* event = IOEvent::Create(IOEvent::SEND, client, packet);
	assert(event);

	StartThreadpoolIo(client->GetTPIO());

	//trace0("send");
	if(WSASend(client->GetSocket(), &recvBufferDescriptor, 1, NULL, sendFlags, &event->GetOverlapped(), NULL) == SOCKET_ERROR)
	{
		int error = WSAGetLastError();

		if(error != ERROR_IO_PENDING)
		{
			CancelThreadpoolIo(client->GetTPIO());

			ERROR_CODE(error, "WSASend() failed.");

			RemoveClient(client);
		}
	}
	else
	{
		// In this case, the completion callback will have already been scheduled to be called.
	}
}


void Server::OnAccept(IOEvent* event)
{
	assert(event);

	//trace("[%d] Enter OnAccept()", GetCurrentThreadId());
	assert(event->GetType() == IOEvent::ACCEPT);

	// Check if we need to post more accept requests.
	InterlockedDecrement(&m_NumPostAccept);

	// Add client in a different thread.
	// It is because we need to return this function ASAP so that this IO worker thread can process the other IO notifications.
	// If adding client is fast enough, we can call it here but I assume it's slow.	
	if(!m_ShuttingDown && TrySubmitThreadpoolCallback(Server::WorkerAddClient, event->GetClient(), &m_ClientTPENV) == false)
	{
		ERROR_CODE(GetLastError(), "Could not start WorkerAddClient.");

		AddClient(event->GetClient());
	}

	//trace("[%d] Leave OnAccept()", GetCurrentThreadId());
}


void Server::OnRecv(IOEvent* event, DWORD dwNumberOfBytesTransfered)
{
	assert(event);
	//trace("[%d] Enter OnRecv()", GetCurrentThreadId());

	Client* pClient = event->GetClient();
	pClient->SetPacket(dwNumberOfBytesTransfered);
	BYTE* pPacket = pClient->GetPacket();
	int nPacketSize = pClient->GetPacketSize();

	UINT command = 6;
	UINT size = 0;
	memcpy_s(&command, sizeof(UINT), pPacket, sizeof(UINT));
	memcpy_s(&size, sizeof(UINT), &pPacket[sizeof(UINT)], sizeof(UINT));

	// WorkerProcessRecvPacket에서 처리를 하므로 의미가 없을 것으로 보임
	// 1. protocol check
	if (command > GFSProtocolHeader::GFSProtocolCommand::UNSOLICITED_EVENT || command < GFSProtocolHeader::GFSProtocolCommand::ERROR_RESPONSE) {
		trace0("Wrong protocol.");
		Log::Trace("This protocol is wrong.");
		pClient->InitPacket();
		PostRecv(pClient);
		return;
	}
	// 2. size check
	if (size <= 0) {
		trace0("size is Zero.");
		Log::Trace("This packet size is zero.");
		pClient->InitPacket();
		PostRecv(pClient);
		return;
	}
	// 3. size check
	if (size > sizeof(GFSProtocolLoginRequest) * 2) {	//현재 클라이언트에서 보내는 패킷 중 가장 큰 Data Size를 갖는 것이 LoginRequest이므로
		trace0("size is too big.");
		Log::Trace("This packet size is too big.");
		pClient->InitPacket();
		PostRecv(pClient);
		return;
	}
	Packet* packet = NULL;
	while (true) {
		if (size > nPacketSize) {
			//trace0("Not enough yet, packet");
			break;
		}
		packet = Packet::Create(pClient, pPacket, size);
		if (!packet) {
			trace0("Packet create failed. command: %d, size: %d", command, size);
			Log::Trace("New packet creation is failed. command: %d, size: %d", command, size);
			break;
		}
		if (TrySubmitThreadpoolCallback(Server::WorkerProcessRecvPacket, packet, NULL) == false)
		{
			ERROR_CODE(GetLastError(), "Could not start WorkerProcessRecvPacket. call it directly.");

			Log::Trace("It could not start WorkerProcessRecvPacket");

			Echo(packet);
		}
		pClient->PacketMove(size);
		pPacket = pClient->GetPacket();
		nPacketSize = pClient->GetPacketSize();
		if (nPacketSize == 0) {
			break;
		}
		memcpy_s(&command, sizeof(UINT), pPacket, sizeof(UINT));
		memcpy_s(&size, sizeof(UINT), &pPacket[sizeof(UINT)], sizeof(UINT));
	}
	PostRecv(pClient);







	//BYTE* buff = event->GetClient()->GetRecvBuff();


	//// recv time check
	//if (m_nRecvSize > 0 && GetTickCount() - m_dwLastRecv > 1000) {
	//	m_nRecvSize = 0;
	//}
	//m_dwLastRecv = GetTickCount();

	//BYTE* buff = event->GetClient()->GetRecvBuff();
	//memcpy_s(&m_recv[m_nRecvSize], dwNumberOfBytesTransfered, buff, dwNumberOfBytesTransfered);
	//m_nRecvSize += dwNumberOfBytesTransfered;

	//USHORT command = 6;
	//int size = 0;
	//memcpy_s(&command, sizeof(USHORT), m_recv, sizeof(USHORT));
	//memcpy_s(&size, sizeof(UINT), &m_recv[sizeof(USHORT)], sizeof(UINT));
	//// protocol check
	//if (command >= ProtocolHeader::DefineEndProtocol || command < ProtocolHeader::Keep_Alive) {
	//	trace0("Wrong protocol.");
	//	m_nRecvSize = 0;
	//	PostRecv(event->GetClient());
	//	return;
	//}
	//// 1. size check
	//if (size <= 0) {
	//	trace0("size is Zero.");
	//	m_nRecvSize = 0;
	//	PostRecv(event->GetClient());
	//	return;
	//}
	//// 2. size check
	//if (size > sizeof(ProtocolRequestAddEvent) * 2) {
	//	trace0("size is too big.");
	//	m_nRecvSize = 0;
	//	PostRecv(event->GetClient());
	//	return;
	//}

	//// Create packet by copying recv buff.
	//UINT nPoint = 0;
	//Packet* packet = NULL;
	//while (true) {
	//	if (size > m_nRecvSize) {
	//		//trace0("Not enough yet, packet");
	//		break;
	//	}
	//	packet = Packet::Create(event->GetClient(), m_recv, size);
	//	if (!packet) {
	//		trace0("Packet create failed. command: %d, size: %d", command, size);
	//		break;
	//	}
	//	if (TrySubmitThreadpoolCallback(Server::WorkerProcessRecvPacket, packet, NULL) == false)
	//	{
	//		ERROR_CODE(GetLastError(), "Could not start WorkerProcessRecvPacket. call it directly.");

	//		Echo(packet);
	//	}
	//	memmove(m_recv, &m_recv[size], m_nRecvSize - size);
	//	m_nRecvSize -= size;
	//	if (m_nRecvSize <= 0) {
	//		m_nRecvSize = 0;
	//		break;
	//	}
	//	memcpy_s(&command, sizeof(USHORT), m_recv, sizeof(USHORT));
	//	memcpy_s(&size, sizeof(UINT), &m_recv[sizeof(USHORT)], sizeof(UINT));
	//}
	//// If whatever game logics relying on the packet are fast enough, we can manage them here but I assume they are slow.	
	//// I think it's better to request receiving ASAP and handle packets received in another thread.
	//PostRecv(event->GetClient());

	//trace("[%d] Leave OnRecv()", GetCurrentThreadId());
}


void Server::OnSend(IOEvent* event, DWORD dwNumberOfBytesTransfered)
{
	assert(event);

	//trace("[%d] OnSend : %d", GetCurrentThreadId(), dwNumberOfBytesTransfered);

	// This should be fast enough to do in this I/O thread.
	// if not, we need to queue it like what we do in OnRecv().
	Packet::Destroy(event->GetPacket());
}


void Server::OnClose(IOEvent* event)
{
	assert(event);

	std::string ip;
	u_short port = 0;
	Network::GetRemoteAddress(event->GetClient()->GetSocket(), ip, port);

	trace0("[%d] Client's socket has been closed : ip[%s], port[%d]", GetCurrentThreadId(), ip.c_str(), port);

	// If whatever game logics about this event are fast enough, we can manage them here but I assume they are slow.	
	if(!m_ShuttingDown && TrySubmitThreadpoolCallback(Server::WorkerRemoveClient, event->GetClient(), &m_ClientTPENV) == false)
	{
		ERROR_CODE(GetLastError(), "can't start WorkerRemoveClient. call it directly.");

		RemoveClient(event->GetClient());
	}
}


void Server::AddClient(Client* client)
{
	assert(client);

// The socket sAcceptSocket does not inherit the properties of the socket associated with sListenSocket parameter until SO_UPDATE_ACCEPT_CONTEXT is set on the socket.
if (setsockopt(client->GetSocket(), SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, reinterpret_cast<const char *>(&m_listenSocket), sizeof(m_listenSocket)) == SOCKET_ERROR)
{
	ERROR_CODE(WSAGetLastError(), "setsockopt() for AcceptEx() failed.");

	RemoveClient(client);
}
else
{
	client->SetState(Client::ACCEPTED);

	// Connect the socket to IOCP
	TP_IO* pTPIO = CreateThreadpoolIo(reinterpret_cast<HANDLE>(client->GetSocket()), Server::IoCompletionCallback, NULL, NULL);
	if (pTPIO == NULL)
	{
		ERROR_CODE(GetLastError(), "CreateThreadpoolIo failed for a client.");

		RemoveClient(client);
	}
	else
	{
		std::string ip;
		u_short port = 0;
		Network::GetRemoteAddress(client->GetSocket(), ip, port);
		trace0("[%d] Accept succeeded. client address : ip[%s], port[%d]", GetCurrentThreadId(), ip.c_str(), port);

		client->InitializePacket(); // create packet buffer
		client->SetTPIO(pTPIO);

		EnterCriticalSection(&m_CSForClients);
		m_Clients.push_back(client);
		LeaveCriticalSection(&m_CSForClients);

		PostRecv(client);
	}
}
}


void Server::RemoveClient(Client* client)
{
	assert(client);

	EnterCriticalSection(&m_CSForClients);

	ClientList::iterator itor = std::remove(m_Clients.begin(), m_Clients.end(), client);

	if (itor != m_Clients.end())
	{
		trace0("[%d] RemoveClient succeeded.", GetCurrentThreadId());

		client->ReleasePacket(); // release packet buffer
		Client::Destroy(client);

		m_Clients.erase(itor);
	}

	LeaveCriticalSection(&m_CSForClients);
}


void Server::Echo(Packet* packet)
{
	assert(packet);
	assert(packet->GetSender());

	EnterCriticalSection(&m_CSForClients);

	ClientList::iterator itor = std::find(m_Clients.begin(), m_Clients.end(), packet->GetSender());

	if (itor == m_Clients.end())
	{
		// No client to send it back.
		Packet::Destroy(packet);
	}
	else
	{
		PostSend(packet->GetSender(), packet);
	}

	LeaveCriticalSection(&m_CSForClients);
}

void Server::Send(Packet* packet, BYTE* pData, int nSize, int nCommand)
{
	assert(packet);
	assert(packet->GetSender());

	EnterCriticalSection(&m_CSForClients);

	ClientList::iterator itor = std::find(m_Clients.begin(), m_Clients.end(), packet->GetSender());

	if (itor == m_Clients.end())
	{
		// No client to send it back.
		Packet::Destroy(packet);
	}
	else
	{
		// 에러 응답이거나 로그인 상태일때만 전송
		if ((nCommand == GFSProtocolHeader::GFSProtocolCommand::ERROR_RESPONSE) || ((*itor)->GetSocket() == Client::LOGGED_IN))
		{
			Packet* pPacket = Packet::Create(packet->GetSender(), pData, nSize);
			if (pPacket) {
				PostSend(pPacket->GetSender(), pPacket);
			}
		}
		
		Packet::Destroy(packet);
	}

	LeaveCriticalSection(&m_CSForClients);
}

void Server::SendAll(BYTE* pData, int nSize)
{
	EnterCriticalSection(&m_CSForClients);

	ClientList::iterator itor;
	for (ClientList::iterator itor = m_Clients.begin(); itor != m_Clients.end(); ++itor)
	{
		//로그인 상태의 client에만 이벤트 전송
		if ((*itor)->GetState() == Client::LOGGED_IN)
		{
			Packet* packet = Packet::Create(itor[0], pData, nSize);
			PostSend(itor[0], packet);
		}
	}

	LeaveCriticalSection(&m_CSForClients);
}

void Server::SendAndSetLoggedInState(Packet* packet, BYTE* pData, int nSize)
{
	assert(packet);
	assert(packet->GetSender());

	EnterCriticalSection(&m_CSForClients);

	ClientList::iterator itor = std::find(m_Clients.begin(), m_Clients.end(), packet->GetSender());

	if (itor == m_Clients.end())
	{
		// No client to send it back.
		Packet::Destroy(packet);
	}
	else
	{
		(*itor)->SetState(Client::LOGGED_IN);
		Packet* pPacket = Packet::Create(packet->GetSender(), pData, nSize);
		if (pPacket) {
			PostSend(pPacket->GetSender(), pPacket);
		}
		Packet::Destroy(packet);
	}

	LeaveCriticalSection(&m_CSForClients);
}

size_t Server::GetNumClients()
{
	EnterCriticalSection(&m_CSForClients);

	size_t num = m_Clients.size();

	LeaveCriticalSection(&m_CSForClients);

	return num;
}

long Server::GetNumPostAccepts()
{
	return m_NumPostAccept;
}

void Server::CheckKeepAliveGFSClients()
{
	EnterCriticalSection(&m_CSForClients);

	ClientList::iterator itor;
	for (ClientList::iterator itor = m_Clients.begin(); itor != m_Clients.end(); ++itor)
	{
		if((*itor)->m_dwLastRecv == 0)
			continue;

		// KeepAlive 주기의 2배로 마진
		if (GetTickCount() - (*itor)->m_dwLastRecv >= TIMER_GFS_SERVER_KEEP_ALIVE_PERIOD * 2)
		{
			Client::Destroy(*itor);
			Log::Trace("KeepAliveRequest is too late!");
		}
	}

	LeaveCriticalSection(&m_CSForClients);
}