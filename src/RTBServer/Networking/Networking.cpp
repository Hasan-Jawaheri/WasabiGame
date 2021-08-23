#include "RTBServer/Networking/Networking.hpp"
#include "RollTheBall/Networking/Protocol.hpp"
#include "RTBServer/Repositories/ClientsRepository.hpp"


RTBServer::ServerNetworking::ServerNetworking(std::shared_ptr<ServerApplication> app, std::shared_ptr<WasabiGame::GameConfig> config, std::shared_ptr<WasabiGame::GameScheduler> scheduler) : WasabiGame::NetworkManager() {
	m_app = app;
	m_listener = std::make_shared<WasabiGame::NetworkListenerT<ServerConnectedClient>>(config, scheduler);
	m_curClientId = 1;
}

uint32_t RTBServer::ServerNetworking::GenerateClientId() {
	return m_curClientId++;
}

void RTBServer::ServerNetworking::Initialize() {
	// client sent new data over TCP, identify message and react to it
	auto onConsumeBuffer = [this](std::shared_ptr<ServerConnectedClient> client, WasabiGame::CircularBuffer* buffer) {
		WasabiGame::NetworkUpdate update;
		size_t size = 1;
		while (size > 0) {
			size = update.readPacket(buffer);
			if (size > 0) {
				// first message from the client must be a login message
				if (update.type != RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOGIN && client->Identity.accountName[0] == 0)
					return false;

				auto it = m_updateCallbacks.find(update.type);
				if (it != m_updateCallbacks.end()) {
					if (!it->second(client, update))
						return false;
				}
			}
		}
		return true;
	};

	// client connected
	m_listener->SetOnClientConnected([this, onConsumeBuffer](std::shared_ptr<WasabiGame::Selectable> _client) {
		std::shared_ptr<ServerConnectedClient> client = std::dynamic_pointer_cast<ServerConnectedClient>(_client);
		{
			std::scoped_lock lockGuard(this->m_clientsMutex);
			client->m_id = this->GenerateClientId();
			client->SetConsumeBufferCallback([client, onConsumeBuffer](WasabiGame::CircularBuffer* buffer) {
				return onConsumeBuffer(client, buffer);
			});
			this->m_clients.insert(std::make_pair(client->m_id, client));
			LOG_F(INFO, "CONNECTED: id=%d", client->m_id);
		}
		
		this->m_app->ClientsRepository->SetClientConnected(client, this->m_app->GetLoginCell());
	});

	// client disconnected
	m_listener->SetOnClientDisconnected([this](std::shared_ptr<WasabiGame::Selectable> _client) {
		std::shared_ptr<ServerConnectedClient> client = std::dynamic_pointer_cast<ServerConnectedClient>(_client);

		this->m_app->ClientsRepository->SetClientDisconnected(client);

		{
			std::scoped_lock lockGuard(this->m_clientsMutex);
			this->m_clients.erase(client->m_id);
			LOG_F(INFO, "DISCONNECTED: id=%d, account=%s", client->m_id, client->Identity.accountName);
		}
	});

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

void RTBServer::ServerNetworking::SendUpdate(std::shared_ptr<WasabiGame::NetworkClient> client, WasabiGame::NetworkUpdate& update, bool important) {
	char packet[WasabiGame::MAX_PACKET_SIZE];
	size_t size = update.fillPacket(packet);

	if (client) {
		if (important)
			client->Write(packet, size);
		//else
		//	m_listener->SendUDPPacket;
	} else {
		std::scoped_lock lockGuard(this->m_clientsMutex);
		for (auto client : m_clients) {
			if (important)
				client.second->Write(packet, size);
			//else
			//	m_listener->SendUDPPacket;
		}
	}
}

void RTBServer::ServerNetworking::SendUpdate(WasabiGame::NetworkUpdate& update, bool important) {
	std::scoped_lock lockGuard(this->m_clientsMutex);
	for (auto client : m_clients)
		SendUpdate(client.second, update, important);
}
