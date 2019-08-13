#include "RTBClient/Networking/Networking.hpp"

#include <WinSock2.h>

size_t RTBNet::NetworkUpdate::fillPacket(char* packet) {
	int hdataSize = dataSize;

	type = htons(type);
	purpose = htons(purpose);
	targetId = htonl(targetId);
	dataSize = htons(dataSize);

	memcpy(packet, (char*)&type, sizeof(NetworkUpdateType));
	memcpy(packet, (char*)&purpose, sizeof(PacketPurpose));
	memcpy(packet, (char*)&targetId, sizeof(uint32_t));
	memcpy(packet, (char*)&dataSize, sizeof(uint16_t));
	memcpy(packet, (char*)data, hdataSize);

	return sizeof(NetworkUpdateType) + sizeof(PacketPurpose) + sizeof(uint32_t);
}

size_t RTBNet::NetworkUpdate::readPacket(HBUtils::CircularBuffer* packet) {
	if (packet->GetSize() >= PACKET_META_SIZE) {
		char tmp[PACKET_META_SIZE];
		size_t availableToRead = min(packet->GetAvailableContigiousConsume(), PACKET_META_SIZE);
		memcpy(tmp, packet->GetMem(), availableToRead);
		if (availableToRead < PACKET_META_SIZE)
			memcpy(tmp + availableToRead, packet->mem, PACKET_META_SIZE - availableToRead);

		type = ntohs(*(NetworkUpdateType*)(tmp + 0));
		purpose = ntohs(*(PacketPurpose*)(tmp + 2));
		targetId = ntohl(*(uint32_t*)(tmp + 4));
		dataSize = ntohs(*(uint16_t*)(tmp + 8));

		if (PACKET_META_SIZE + dataSize >= packet->GetSize()) {
			packet->OnConsumed(PACKET_META_SIZE);
			size_t dataRead = 0;
			while (dataRead < dataSize) {
				size_t sizeToRead = min(packet->GetAvailableContigiousConsume(), dataSize - dataRead);
				memcpy(data + dataRead, packet->GetMem(), sizeToRead);
				packet->OnConsumed(sizeToRead);
				dataRead += sizeToRead;
			}
			return PACKET_META_SIZE + dataSize;
		}
	}
	return 0;
}

RTBNet::RTBClientNetworking::RTBClientNetworking() {
	m_server = new RPGNet::ServerT<RPGNet::Client>();
	m_tcpConnection = new RPGNet::ReconnectingClient(m_server);
	m_udpConnection = new RPGNet::ReconnectingClient(m_server);
	Status = CONNECTION_NOT_CONNECTED;
}

void RTBNet::RTBClientNetworking::Initialize() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	m_server->Config.Set<int>("tcpPort", 0);
	m_server->Config.Set<int>("udpPort", 0);
	m_server->Config.Set<int>("numWorkers", 1);

	std::function<bool(HBUtils::CircularBuffer*)> onConsumeBuffer = [this](HBUtils::CircularBuffer* buffer) {
		RTBNet::NetworkUpdate update;
		size_t size = update.readPacket(buffer);
		if (size > 0) {
			auto it = m_updateCallbacks.find(update.type);
			if (it != m_updateCallbacks.end())
				it->second(update);
		}
		return true;
	};

	m_tcpConnection->SetConsumeBufferCallback(onConsumeBuffer);
	m_udpConnection->SetConsumeBufferCallback(onConsumeBuffer);

	m_networkingThread = new std::thread([this]() {
		this->m_server->Run();
	});
}

void RTBNet::RTBClientNetworking::Destroy() {
	m_server->Stop();
	m_networkingThread->join();
	delete m_networkingThread;
}

void RTBNet::RTBClientNetworking::Login() {
	Status = CONNECTION_CONNECTING;
	if (m_tcpConnection->Connect("127.0.0.1", 9987) == 0)
		Status = CONNECTION_CONNECTED;
	else
		Status = CONNECTION_NOT_CONNECTED;
}

void RTBNet::RTBClientNetworking::Logout() {
	m_tcpConnection->Close();
	Status = CONNECTION_NOT_CONNECTED;
}

void RTBNet::RTBClientNetworking::SendUpdate(NetworkUpdate& update, bool important) {
	char packet[RTBNet::MAX_PACKET_SIZE];
	size_t size = update.fillPacket(packet);

	if (important)
		m_tcpConnection->Write(packet, size);
	else
		m_udpConnection->Write(packet, size);
}

void RTBNet::RTBClientNetworking::RegisterNetworkUpdateCallback(NetworkUpdateType type, std::function<void(NetworkUpdate&)> callback) {
	auto it = m_updateCallbacks.find(type);
	if (it != m_updateCallbacks.end())
		m_updateCallbacks.erase(it);
	m_updateCallbacks.insert(std::make_pair(type, callback));
}

void RTBNet::RTBClientNetworking::ClearNetworkUpdateCallback(NetworkUpdateType type) {
	auto it = m_updateCallbacks.find(type);
	if (it != m_updateCallbacks.end())
		m_updateCallbacks.erase(it);
}
