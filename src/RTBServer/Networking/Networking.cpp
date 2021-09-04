#include "RTBServer/Networking/Networking.hpp"
#include "RollTheBall/Networking/Protocol.hpp"
#include "RTBServer/Repositories/ClientsRepository.hpp"


RTBServer::ServerNetworking::ServerNetworking(std::shared_ptr<ServerApplication> app, std::shared_ptr<WasabiGame::GameConfig> config, std::shared_ptr<WasabiGame::GameScheduler> scheduler) : WasabiGame::NetworkManager() {
	m_app = app;
	m_listener = std::make_shared<WasabiGame::NetworkListenerT<ServerConnectedClient>>(config, scheduler);
	m_UDPClient = std::make_shared<WasabiGame::UDPServerSideNetworkClient>(m_listener);
	m_curClientId = 1;
}

uint32_t RTBServer::ServerNetworking::GenerateClientId() {
	return m_curClientId++;
}

void RTBServer::ServerNetworking::Initialize() {
	// UDP packet read
	m_UDPClient->SetConsumeUDPPacketCallback([this](void* packet, size_t length, sockaddr* addrFrom, int addrLen) { this->OnReceivedUDPPacket(packet, length, addrFrom, addrLen); });
	m_listener->SetOnClientConnected([this](std::shared_ptr<WasabiGame::Selectable> client) { this->OnClientConnected(client); });
	m_listener->SetOnClientDisconnected([this](std::shared_ptr<WasabiGame::Selectable> client) { this->OnClientDisconnected(client); });

	m_listener->RunDetached();
}

void RTBServer::ServerNetworking::Destroy() {
	m_app = nullptr;
	m_listener->SetOnClientConnected(nullptr);
	m_listener->SetOnClientDisconnected(nullptr);
	m_listener->Stop();
}

std::shared_ptr<WasabiGame::NetworkListenerT<RTBServer::ServerConnectedClient>> RTBServer::ServerNetworking::GetListener() const {
	return m_listener;
}

void RTBServer::ServerNetworking::SendUpdate(uint32_t clientId, WasabiGame::NetworkUpdate& update, bool important) {
	std::shared_ptr<ServerConnectedClient> client = nullptr;
	if (clientId != 0) {
		std::scoped_lock lockGuard(this->m_clientsMutex);
		auto it = m_clients.find(clientId);
		if (it != m_clients.end())
			client = it->second;
	}

	if (client || clientId == 0)
		SendUpdate(client, update, important);
}

void RTBServer::ServerNetworking::SendUpdate(std::shared_ptr<WasabiGame::NetworkClient> _client, WasabiGame::NetworkUpdate& update, bool important) {
	char packet[WasabiGame::MAX_PACKET_SIZE];
	size_t size = update.fillPacket(packet);
	std::shared_ptr<ServerConnectedClient> client = std::dynamic_pointer_cast<ServerConnectedClient>(_client);

	if (client) {
		if (important)
			client->Write(packet, size);
		else if (client->m_udpAddrLen > 0) // ignore UDP packets if client hasn't connected with UDP yet
			m_UDPClient->Write(packet, size, &client->m_udpAddr, client->m_udpAddrLen);
	} else {
		std::vector<std::shared_ptr<ServerConnectedClient>> allClients;
		allClients.reserve(m_clients.size());
		{
			std::scoped_lock lockGuard(this->m_clientsMutex);
			for (auto client : m_clients) {
				allClients.push_back(client.second);
			}
		}

		for (auto client : allClients) {
			if (important)
				client->Write(packet, size);
			else if (client->m_udpAddrLen > 0) // ignore UDP packets if client hasn't connected with UDP yet
				m_UDPClient->Write(packet, size, &client->m_udpAddr, client->m_udpAddrLen);
		}
	}
}

void RTBServer::ServerNetworking::SendUpdate(WasabiGame::NetworkUpdate& update, bool important) {
	SendUpdate(nullptr, update, important);
}

void RTBServer::ServerNetworking::OnClientConnected(std::shared_ptr<WasabiGame::Selectable> _client) {
	std::shared_ptr<ServerConnectedClient> client = std::dynamic_pointer_cast<ServerConnectedClient>(_client);
	{
		std::scoped_lock lockGuard(m_clientsMutex);
		client->m_id = this->GenerateClientId();
		client->SetConsumeBufferCallback([this, client](char* buffer, size_t length) {
			// client sent new data over TCP, identify message and react to it
			WasabiGame::NetworkUpdate packet;
			size_t size = packet.readPacket(buffer, length);
			if (size > 0)
				return this->OnReceivedNetworkUpdate(client, packet);
			return false;
		});
		m_clients.insert(std::make_pair(client->m_id, client));
		LOG_F(INFO, "CLIENT CONNECTED: id=%d", client->m_id);
	}

	// send client id to the client that just connected
	WasabiGame::NetworkUpdate setClientIdUpdate;
	RollTheBall::UpdateBuilders::SetClientId(setClientIdUpdate, client->m_id);
	SendUpdate(client, setClientIdUpdate, true);

	this->m_app->ClientsRepository->SetClientConnected(client, this->m_app->GetLoginCell());
}

void RTBServer::ServerNetworking::OnClientDisconnected(std::shared_ptr<WasabiGame::Selectable> _client) {
	std::shared_ptr<ServerConnectedClient> client = std::dynamic_pointer_cast<ServerConnectedClient>(_client);

	this->m_app->ClientsRepository->SetClientDisconnected(client);

	{
		std::scoped_lock lockGuard(m_clientsMutex);
		if (client->m_udpAddrLen > 0)
			m_udpPortToClientId.erase(((sockaddr_in*)&client->m_udpAddr)->sin_port);
		m_clients.erase(client->m_id);
		LOG_F(INFO, "CLIENT DISCONNECTED: id=%d, account=%s", client->m_id, client->Identity.accountName);
	}
}

bool RTBServer::ServerNetworking::OnReceivedNetworkUpdate(std::shared_ptr<ServerConnectedClient> client, WasabiGame::NetworkUpdate update) {
	// first message from the client must be a login message
	if (static_cast<RollTheBall::NetworkUpdateTypeEnum>(update.type) != RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOGIN && client->Identity.accountName[0] == 0)
		return false;

	auto it = m_updateCallbacks.find(update.type);
	if (it != m_updateCallbacks.end()) {
		if (!it->second(client, update))
			return false;
	}

	return true;
}

void RTBServer::ServerNetworking::OnReceivedUDPPacket(void* packet, size_t length, sockaddr* addrFrom, int addrLen) {
	WasabiGame::NetworkUpdate update;
	size_t size = update.readPacket(packet, length);
	if (size > 0) {
		uint32_t portFrom = ((sockaddr_in*)addrFrom)->sin_port;
		std::shared_ptr<ServerConnectedClient> client = nullptr;
		auto clientIdIter = m_udpPortToClientId.find(portFrom);
		if (clientIdIter == m_udpPortToClientId.end()) {
			// don't know which client this udp PoRT maps to, this packet will only be accepted if it identifies sender's id
			if (static_cast<RollTheBall::NetworkUpdateTypeEnum>(update.type) == RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_IDENTIFY_UDP_CLIENT) {
				{
					std::scoped_lock lockGuard(m_clientsMutex);
					auto clientsIter = m_clients.find(update.targetId);
					if (clientsIter != m_clients.end()) {
						if (clientsIter->second->m_udpAddrLen == 0) {
							client = clientsIter->second;
							client->m_udpAddr = *addrFrom;
							client->m_udpAddrLen = addrLen;
							m_udpPortToClientId.insert(std::make_pair(portFrom, client->m_id));
						}
					}
				}
				if (client) {
					// client has been identified for the first time, ACK that we identified client (over TCP)
					WasabiGame::NetworkUpdate ackUpdate;
					RollTheBall::UpdateBuilders::UDPClientIdentified(ackUpdate);
					SendUpdate(client, ackUpdate, true);
				}
			}
		}

		if (!client && clientIdIter != m_udpPortToClientId.end()) {
			std::scoped_lock lockGuard(m_clientsMutex);
			auto clientsIter = m_clients.find(clientIdIter->second);
			if (clientsIter != m_clients.end())
				client = clientsIter->second;
		}

		if (client)
			OnReceivedNetworkUpdate(client, update);
	}
}
