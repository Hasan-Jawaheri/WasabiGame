#include "RTBClient/Networking/Networking.hpp"
#include "WasabiGame/Main.hpp"
#include "Wasabi/Core/WTimer.hpp"

#include <random>

RTBClient::ClientNetworking::ClientNetworking(std::shared_ptr<WasabiGame::WasabiBaseGame> app, std::shared_ptr<WasabiGame::GameConfig> config, std::shared_ptr<WasabiGame::GameScheduler> scheduler)
		: WasabiGame::NetworkManager() {
	m_timer = new WTimer();
	m_listener = std::make_shared<WasabiGame::NetworkListenerT<WasabiGame::NetworkClient>>(config, scheduler);
	m_tcpConnection = std::make_shared<WasabiGame::ReconnectingNetworkClient>(m_listener);
	m_udpConnection = std::make_shared<WasabiGame::UDPNetworkClient>(m_listener);
	m_serverTCPPort = config->Get<int>("tcpPort");
	m_serverUDPPort = config->Get<int>("udpPort");
	Status = RTBConnectionStatus::CONNECTION_NOT_CONNECTED;
	m_clientId = 0; // will receive it from the server
	m_lastUDPIdentificationUpdate = -10000.0f;
	m_lastLoginUpdate = -10000.0f;
}

void RTBClient::ClientNetworking::Initialize() {
	WSADATA wsa;
	(void)WSAStartup(MAKEWORD(2, 2), &wsa);

	// this will cause the listener's selectables-loop to not make TCP/UDP servers
	m_listener->Config->Set<int>("tcpPort", 0);
	m_listener->Config->Set<int>("udpPort", 0);
	m_listener->Config->Set<int>("numWorkers", 1);

	std::function<bool(char*, size_t)> onConsumeBuffer = [this](char* buffer, size_t length) {
		WasabiGame::NetworkUpdate update;
		size_t size = update.readPacket(buffer, length);
		if (size == 0)
			return false;
		auto it = m_updateCallbacks.find(update.type);
		if (it != m_updateCallbacks.end())
			it->second(nullptr, update);
		return true;
	};

	m_tcpConnection->SetConsumeBufferCallback(onConsumeBuffer);
	m_udpConnection->SetConsumeBufferCallback(onConsumeBuffer);

	m_tcpConnection->SetOnConnectingCallback([this]() {
		this->Status = RTBConnectionStatus::CONNECTION_CONNECTING;
	});
	
	m_tcpConnection->SetOnConnectedCallback([this]() {
		m_clientId = 0; // will receive it from the server in UPDATE_TYPE_SET_CLIENT_ID
		this->Status = RTBConnectionStatus::CONNECTION_HANDSHAKING;
	});

	RegisterNetworkUpdateCallback(static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_SET_CLIENT_ID),
		[this](std::shared_ptr<WasabiGame::Selectable> selectable, WasabiGame::NetworkUpdate& update) {
			this->m_clientId = update.targetId;
			return true;
		}
	);

	RegisterNetworkUpdateCallback(static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_UDP_CLIENT_IDENTIFIED),
		[this](std::shared_ptr<WasabiGame::Selectable> selectable, WasabiGame::NetworkUpdate& update) {
			this->Status = RTBConnectionStatus::CONNECTION_AUTHENTICATING;
			return true;
		}
	);

	RegisterNetworkUpdateCallback(static_cast<WasabiGame::NetworkUpdateType>(RollTheBall::NetworkUpdateTypeEnum::UPDATE_TYPE_LOGIN_STATUS),
		[this](std::shared_ptr<WasabiGame::Selectable> selectable, WasabiGame::NetworkUpdate& update) {
			bool success;
			RollTheBall::UpdateBuilders::ReadLoginStatusPacket(update, success);
			if (success)
				this->Status = RTBConnectionStatus::CONNECTION_CONNECTED;
			return true;
		}
	);

	m_tcpConnection->SetOnDisconnectedCallback([this]() {
		this->Status = RTBConnectionStatus::CONNECTION_NOT_CONNECTED;
	});

	m_tcpConnection->SetOnConnectionFailedCallback([this]() {
		this->Status = RTBConnectionStatus::CONNECTION_NOT_CONNECTED;
	});

	m_timer->Start();
	m_listener->RunDetached();
}

void RTBClient::ClientNetworking::Destroy() {
	Logout();
	m_listener->SetOnClientConnected(nullptr);
	m_listener->SetOnClientDisconnected(nullptr);
	m_listener->Stop();
}

void RTBClient::ClientNetworking::Update(float fDeltaTime) {
	float elapsedTime = m_timer->GetElapsedTime(true);
	if (Status == RTBConnectionStatus::CONNECTION_HANDSHAKING && m_clientId > 0 && m_lastUDPIdentificationUpdate + 5000.0 < elapsedTime) {
		// send identification over UDP
		WasabiGame::NetworkUpdate update;
		RollTheBall::UpdateBuilders::IdentifyUDPClient(update, m_clientId);
		SendUpdate(update, false);
		m_lastUDPIdentificationUpdate = elapsedTime;
	}

	if (Status == RTBConnectionStatus::CONNECTION_AUTHENTICATING && m_lastLoginUpdate + 10000.0 < elapsedTime) {
		// send a login update
		WasabiGame::NetworkUpdate loginUpdate;
		RollTheBall::UpdateBuilders::Login(loginUpdate, ("ghandi-" + std::to_string(std::rand() % 10000)).c_str(), "123456");
		SendUpdate(loginUpdate);
		m_lastLoginUpdate = elapsedTime;
	}
}

void RTBClient::ClientNetworking::Login() {
	m_tcpConnection->Connect("127.0.0.1", m_serverTCPPort);
	m_udpConnection->Connect("127.0.0.1", m_serverUDPPort);
}

void RTBClient::ClientNetworking::Logout() {
	m_tcpConnection->StopReconnecting();
	m_udpConnection->Close();
}

void RTBClient::ClientNetworking::SendUpdate(std::shared_ptr<WasabiGame::NetworkClient> client, WasabiGame::NetworkUpdate& update, bool important) {
	SendUpdate(update, important);
}

void RTBClient::ClientNetworking::SendUpdate(uint32_t clientId, WasabiGame::NetworkUpdate& update, bool important) {
	SendUpdate(update, important);
}

void RTBClient::ClientNetworking::SendUpdate(WasabiGame::NetworkUpdate& update, bool important) {
	char packet[WasabiGame::MAX_PACKET_SIZE];
	size_t size = update.fillPacket(packet);

	if (important)
		m_tcpConnection->Write(packet, size);
	else
		m_udpConnection->Write(packet, size);
}
