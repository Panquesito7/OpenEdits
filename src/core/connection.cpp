#include "connection.h"
#include "packet.h"

#include <enet/enet.h>
#include <iostream>
#include <pthread.h>
#include <string.h> // strerror
#include <sstream>

#if 0
	#define DEBUGLOG(...) printf(__VA_ARGS__)
#else
	#define DEBUGLOG(...) /* SILENCE */
#endif
#define ERRORLOG(...) fprintf(stderr, __VA_ARGS__)

const uint16_t CON_PORT = 0xC014;
const size_t CON_CLIENTS = 32; // server only
const size_t CON_CHANNELS = 2;

struct enet_init {
	enet_init()
	{
		if (enet_initialize() != 0)
			std::terminate();

		puts("--> ENet start");
	}
	~enet_init()
	{
		puts("<-- ENet stop");
		enet_deinitialize();
	}
} ENET_INIT;

Connection::Connection(Connection::ConnectionType type, const char *name)
{
	if (name)
		m_name = name;

	if (type == TYPE_CLIENT) {
		DEBUGLOG("--- ENet %s: Initializing client\n", m_name);
		m_host = enet_host_create(
			NULL,
			1, // == server
			CON_CHANNELS,
			0, // unlimited incoming bandwidth
			0 // unlimited outgoing bandwidth
		);
	}

	if (type == TYPE_SERVER) {
		ENetAddress address;
		address.host = ENET_HOST_ANY;
		address.port = CON_PORT;

		DEBUGLOG("--- ENet %s: Starting server on port %d\n", m_name, address.port);
		m_host = enet_host_create(
			&address,
			CON_CLIENTS,
			CON_CHANNELS,
			0, // unlimited incoming bandwidth
			0 // unlimited outgoing bandwidth
		);
	}

	if (!m_host) {
		ERRORLOG("-!- ENet %s: Failed to initialize instance\n", m_name);
	}
}

Connection::~Connection()
{
	m_running = false;

	if (m_thread) {
		pthread_join(m_thread, nullptr);
		m_thread = 0;
	}

	// Apply force if needed
	for (size_t i = 0; i < m_host->peerCount; ++i)
		enet_peer_reset(&m_host->peers[i]);

	enet_host_destroy(m_host);
}

// -------------- Public members -------------

bool Connection::connect(const char *hostname)
{
	ENetAddress address;
	enet_address_set_host(&address, hostname);
	address.port = CON_PORT;

	ENetPeer *peer = enet_host_connect(m_host, &address, 2, 0);
	if (!peer) {
		ERRORLOG("-!- ENet: No free peers\n");
		return false;
	}

	char name[100];
	enet_address_get_host(&address, name, sizeof(name));
	printf("--- ENet: Connected to %s:%u\n", name, address.port);
	return true;
}


void Connection::flush()
{
	enet_host_flush(m_host);
}

size_t Connection::getPeerIDs(std::vector<peer_t> *fill) const
{
	if (fill)
		fill->clear();

	size_t count = 0;
	for (size_t i = 0; i < m_host->peerCount; ++i) {
		if (m_host->peers[i].state == ENET_PEER_STATE_CONNECTED) {
			if (fill)
				fill->push_back(m_host->peers[i].connectID);
			count++;
		}
	}
	return count;
}

bool Connection::listenAsync(PacketProcessor &proc)
{
	m_running = true;

	int status = pthread_create(&m_thread, nullptr, &recvAsync, this);

	if (status != 0) {
		m_running = false;
		ERRORLOG("-!- ENet: Failed to start pthread: %s\n", strerror(status));
		return false;
	}

	m_processor = &proc;
	return m_running;
}

void Connection::disconnect(peer_t peer_id)
{
	auto peer = findPeer(peer_id);
	if (!peer)
		return;

	enet_peer_disconnect(peer, 0);
	// Actual handling done in async event handler
}

void Connection::send(peer_t peer_id, uint16_t flags, Packet &pkt)
{
	uint8_t channel = flags & FLAG_MASK_CHANNEL;

	// Most of it should be reliable
	// TODO: test ENET_PACKET_FLAG_UNSEQUENCED
	if (!(flags & FLAG_UNRELIABLE))
		pkt.data()->flags |= ENET_PACKET_FLAG_RELIABLE;

	if (flags & FLAG_BROADCAST) {
		peer_id = 0;
		enet_host_broadcast(m_host, channel, pkt.data());
	} else {
		auto peer = findPeer(peer_id);
		peer_id = peer->connectID;
		enet_peer_send(peer, channel, pkt.data());
	}

	DEBUGLOG("--- ENet: packet sent. peer_id=%u, channel=%d, dump=%s\n",
		peer_id, (int)channel, pkt.dump().c_str());
}

// -------------- Private members -------------

void *Connection::recvAsync(void *con_p)
{
	Connection *con = (Connection *)con_p;
	con->recvAsyncInternal();

	printf("<-- ENet: Thread stop\n");
	return nullptr;
}

void Connection::recvAsyncInternal()
{
	int shutdown_seen = 0;
	ENetEvent event;

	std::vector<peer_t> peer_id_list(m_host->peerCount, 0);

	while (true) {
		if (!m_running && shutdown_seen == 0) {
			shutdown_seen++;
			DEBUGLOG("--- ENet: Shutdown requested\n");

			// Lazy disconnect
			SimpleLock lock(m_peers_lock);
			for (size_t i = 0; i < m_host->peerCount; ++i)
				enet_peer_disconnect_later(&m_host->peers[i], 0);
		}

		if (enet_host_service(m_host, &event, 500) <= 0) {
			// Abort after 2 * 1000 ms
			if (shutdown_seen > 0) {
				if (++shutdown_seen > 2)
					break;
			}

			continue;
		}

		switch (event.type) {
			case ENET_EVENT_TYPE_CONNECT:
				{
					peer_t peer_id = event.peer->connectID;
					DEBUGLOG("--- ENet %s: New peer ID %u\n", m_name, peer_id);
					peer_id_list[event.peer - m_host->peers] = peer_id;
					m_processor->onPeerConnected(peer_id);
				}
				break;
			case ENET_EVENT_TYPE_RECEIVE:
				{
					peer_t peer_id = event.peer->connectID;
					DEBUGLOG("--- ENet %s: Got packet peer_id=%u, channel=%d, len=%zu\n",
						m_name, peer_id,
						(int)event.channelID,
						event.packet->dataLength
					);

					Packet pkt(&event.packet);
					try {
						m_processor->processPacket(peer_id, pkt);
					} catch (std::exception &e) {
						ERRORLOG("--- ENet %s: Unhandled exception while processing packet %s: %s\n",
							m_name, pkt.dump().c_str(), e.what());
					}
				}
				break;
			case ENET_EVENT_TYPE_DISCONNECT:
				{
					// event.peer->connectID is always 0. Need to cache it.
					peer_t peer_id = peer_id_list.at(event.peer - m_host->peers);
					DEBUGLOG("--- ENet %s: Peer %u disconnected\n", m_name, peer_id);
					m_processor->onPeerDisconnected(peer_id);
				}
				break;
			case ENET_EVENT_TYPE_NONE:
				puts("How did I end up here?");
				break;
		}
	}
}

_ENetPeer *Connection::findPeer(peer_t peer_id)
{
	SimpleLock lock(m_peers_lock);

	for (size_t i = 0; i < m_host->peerCount; ++i) {
		if (m_host->peers[i].state != ENET_PEER_STATE_CONNECTED)
			continue;

		peer_t current = m_host->peers[i].connectID;
		if (current == peer_id || peer_id == PEER_ID_FIRST)
			return &m_host->peers[i];
	}

	throw std::runtime_error((std::stringstream() << "Cannot find peer_id=" << peer_id).str());
}

