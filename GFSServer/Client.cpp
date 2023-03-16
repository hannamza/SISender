#include "stdafx.h"
#include "Client.h"
#include "Log.h"
#include "Network.h"

#include <boost/pool/singleton_pool.hpp>


// use thread-safe memory pool
typedef boost::singleton_pool<Client, sizeof(Client)> ClientPool;

void Client::InitializePacket()
{
	m_dwLastRecv = 0;
	m_nPacketPoint = 0;
	m_pPacket = new BYTE[MAX_PACKET_BUFFER];
	memset(m_pPacket, 0x00, MAX_PACKET_BUFFER);
}

void Client::ReleasePacket()
{
	SAFE_DELETE(m_pPacket);
}

/* static */ Client* Client::Create()
{
	Client* client = NULL;
	client = static_cast<Client*>(ClientPool::malloc());
	if (!client) {
		return NULL;
	}
	client->m_State = WAIT;

	client->m_Socket = Network::CreateSocket(false, 0);
	if(client->m_Socket == INVALID_SOCKET)
	{
		ERROR_MSG("Could not create socket.");
		ClientPool::free(client);
		return NULL;
	}
	return client;
}


/* static */ void Client::Destroy(Client* client)
{
	if( client->m_Socket != INVALID_SOCKET )
	{
		Network::CloseSocket(client->m_Socket);
		CancelIoEx(reinterpret_cast<HANDLE>(client->m_Socket), NULL);
		client->m_Socket = INVALID_SOCKET;
		client->m_State = DISCONNECTED;
	}

	if( client->m_pTPIO != NULL )
	{
		WaitForThreadpoolIoCallbacks( client->m_pTPIO, true );
		CloseThreadpoolIo( client->m_pTPIO );
		client->m_pTPIO = NULL;
	}

	ClientPool::free(client);
}

/* static */ void Client::Destroy()
{
	ClientPool::purge_memory();
}

bool Client::SetPacket(int nSize)
{
	if (m_pPacket == NULL) {
		m_pPacket = new BYTE[MAX_PACKET_BUFFER];
	}
	if (GetTickCount() - m_dwLastRecv > 1000) {
		InitPacket();
	}
	if (m_nPacketPoint + nSize > MAX_PACKET_BUFFER) {
		InitPacket();
		return false;
	}
	memcpy_s(&m_pPacket[m_nPacketPoint], nSize, m_recvBuffer, nSize);
	m_nPacketPoint += nSize;

	m_dwLastRecv = GetTickCount();
}

void Client::InitPacket()
{
	m_nPacketPoint = 0;
}

void Client::PacketMove(int nSize)
{
	if (m_pPacket == NULL) {
		m_pPacket = new BYTE[MAX_PACKET_BUFFER];
	}
	memmove(m_pPacket, &m_pPacket[nSize], m_nPacketPoint - nSize);
	m_nPacketPoint -= nSize;
	if (m_nPacketPoint < 0) {
		m_nPacketPoint = 0;
	}
}