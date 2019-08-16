#include "RTBServer/Networking/Networking.hpp"

RTBNet::RTBServerNetworking::RTBServerNetworking() {
	m_game = nullptr;
	m_server = new RPGNet::ServerT<RTBServerConnectedClient>();
	m_curClientId = 1;
}

uint32_t RTBNet::RTBServerNetworking::GenerateClientId() {
	return m_curClientId++;
}

void RTBNet::RTBServerNetworking::Initialize(RTBGame* game) {
	m_game = game;

	WSADATA wsa;
	(void)WSAStartup(MAKEWORD(2, 2), &wsa);

	// client sent new data over TCP
	auto onConsumeBuffer = [this](RTBServerConnectedClient* client, HBUtils::CircularBuffer* buffer) {
		RPGNet::NetworkUpdate update;
		size_t size = update.readPacket(buffer);
		if (size > 0) {
			if (update.type != RTBNet::UpdateTypeEnum::UPDATE_TYPE_LOGIN && client->Identity.accountName[0] == 0)
				return false;

			auto it = m_updateCallbacks.find(update.type);
			if (it != m_updateCallbacks.end())
				return it->second(client, update);
		}
		return true;
	};

	// client connected
	m_server->SetOnClientConnected([this, onConsumeBuffer](RPGNet::Selectable* _client) {
		RTBServerConnectedClient* client = (RTBServerConnectedClient*)_client;
		{
			std::lock_guard lockGuard(this->m_clientsMutex);
			client->m_id = this->GenerateClientId();
			client->SetConsumeBufferCallback([client, onConsumeBuffer](HBUtils::CircularBuffer* buffer) {
				return onConsumeBuffer(client, buffer);
			});
			this->m_clients.insert(std::make_pair(client->m_id, client));
		}
	});

	// client disconnected
	m_server->SetOnClientDisconnected([this](RPGNet::Selectable* _client) {
		RTBServerConnectedClient* client = (RTBServerConnectedClient*)_client;

		m_game->OnClientDisconnected(client);

		{
			std::lock_guard lockGuard(this->m_clientsMutex);
			this->m_clients.erase(client->m_id);
		}
	});

	// login update callback
	RegisterNetworkUpdateCallback(RTBNet::UpdateTypeEnum::UPDATE_TYPE_LOGIN, [this](RTBServerConnectedClient* client, RPGNet::NetworkUpdate& loginUpdate) {
		RPGNet::ClientIdentity identity;
		if (RTBNet::UpdateBuilders::ReadLoginPacket(loginUpdate, identity)) {
			if (this->Authenticate(identity)) {
				memcpy(&client->Identity, &identity, sizeof(RPGNet::ClientIdentity));
				m_game->OnClientConnected(client);
			} else
				return false;
		}
		return true;
	});
}

void RTBNet::RTBServerNetworking::Destroy() {
	m_game = nullptr;
	m_server->Stop();
	delete m_server;
}

RPGNet::ServerT<RTBNet::RTBServerConnectedClient>* RTBNet::RTBServerNetworking::GetServer() const {
	return m_server;
}

void RTBNet::RTBServerNetworking::SendUpdate(uint32_t clientId, RPGNet::NetworkUpdate& update, bool important) {
	RTBServerConnectedClient* client = nullptr;
	{
		std::lock_guard lockGuard(this->m_clientsMutex);
		auto it = m_clients.find(clientId);
		if (it != m_clients.end())
			client = it->second;
	}

	if (client)
		SendUpdate(client, update, important);
}

void RTBNet::RTBServerNetworking::SendUpdate(RTBNet::RTBServerConnectedClient* client, RPGNet::NetworkUpdate& update, bool important) {
	char packet[RPGNet::MAX_PACKET_SIZE];
	size_t size = update.fillPacket(packet);

	if (important)
		client->Write(packet, size);
	//else
	//	m_server->SendUDPPacket;
}

void RTBNet::RTBServerNetworking::RegisterNetworkUpdateCallback(RPGNet::NetworkUpdateType type, std::function<bool(RTBServerConnectedClient*, RPGNet::NetworkUpdate&)> callback) {
	auto it = m_updateCallbacks.find(type);
	if (it != m_updateCallbacks.end())
		m_updateCallbacks.erase(it);
	m_updateCallbacks.insert(std::make_pair(type, callback));
}

void RTBNet::RTBServerNetworking::ClearNetworkUpdateCallback(RPGNet::NetworkUpdateType type) {
	auto it = m_updateCallbacks.find(type);
	if (it != m_updateCallbacks.end())
		m_updateCallbacks.erase(it);
}

bool RTBNet::RTBServerNetworking::Authenticate(RPGNet::ClientIdentity& identity) {
	return std::string(identity.accountName).find("ghandi") == 0 && std::string(identity.passwordHash) == "123456";
}
