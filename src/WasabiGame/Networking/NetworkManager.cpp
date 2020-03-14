#include "WasabiGame/Networking/NetworkManager.hpp"
#include "WasabiGame/Main.hpp"


WasabiGame::NetworkManager::NetworkManager() : NetworkManager(nullptr) {
}

WasabiGame::NetworkManager::NetworkManager(std::shared_ptr<WasabiGame::WasabiBaseGame> app) : std::enable_shared_from_this<NetworkManager>() {
	m_app = app;
}

WasabiGame::NetworkManager::~NetworkManager() {

}

void WasabiGame::NetworkManager::RegisterNetworkUpdateCallback(WasabiGame::NetworkUpdateType type, std::function<bool(std::shared_ptr<Selectable>, WasabiGame::NetworkUpdate&)> callback) {
	auto it = m_updateCallbacks.find(type);
	if (it != m_updateCallbacks.end())
		m_updateCallbacks.erase(it);
	m_updateCallbacks.insert(std::make_pair(type, callback));
}

void WasabiGame::NetworkManager::ClearNetworkUpdateCallback(WasabiGame::NetworkUpdateType type) {
	auto it = m_updateCallbacks.find(type);
	if (it != m_updateCallbacks.end())
		m_updateCallbacks.erase(it);
}

