#include "stdafx.h"
#include "Packet.h"

#include <boost/pool/singleton_pool.hpp>


typedef boost::singleton_pool<Packet, sizeof(Packet)> PacketPool;

/* static */ Packet* Packet::Create(Client* sender, const BYTE* buff, DWORD size)
{
	Packet* packet = NULL;
	packet = static_cast<Packet*>(PacketPool::malloc());
	if (!packet) {
		return NULL;
	}
	packet->m_Sender = sender; 
	packet->m_Size = size;
	CopyMemory(packet->m_Data, buff, size);

	return packet;
}

/* static */ void Packet::Destroy(Packet* packet)
{
	PacketPool::free(packet);
}

/* static */ void Packet::Destroy()
{
	PacketPool::purge_memory();
}