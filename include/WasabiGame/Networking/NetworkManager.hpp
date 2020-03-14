#pragma once

#include "WasabiGame/Networking/Data.hpp"
#include "WasabiGame/Networking/NetworkClient.hpp"
#include "WasabiGame/Networking/NetworkListener.hpp"


namespace WasabiGame {

	class WasabiBaseGame;

	class NetworkManager : public std::enable_shared_from_this<NetworkManager> {

	protected:
		std::shared_ptr<WasabiGame::WasabiBaseGame> m_app;
		std::unordered_map<WasabiGame::NetworkUpdateType, std::function<bool(std::shared_ptr<Selectable>, WasabiGame::NetworkUpdate&)>> m_updateCallbacks;

	public:
		NetworkManager();
		NetworkManager(std::shared_ptr<WasabiGame::WasabiBaseGame> app);
		virtual ~NetworkManager();
		
		virtual void Initialize() = 0;
		virtual void Destroy() = 0;
		virtual void Update(float fDeltaTime) {}

		void RegisterNetworkUpdateCallback(WasabiGame::NetworkUpdateType type, std::function<bool(std::shared_ptr<Selectable>, WasabiGame::NetworkUpdate&)> callback);
		void ClearNetworkUpdateCallback(WasabiGame::NetworkUpdateType type);

		virtual void SendUpdate(std::shared_ptr<NetworkClient> client, WasabiGame::NetworkUpdate& update, bool important = true) = 0;
		virtual void SendUpdate(uint32_t clientId, WasabiGame::NetworkUpdate& update, bool important = true) = 0;
		virtual void SendUpdate(WasabiGame::NetworkUpdate& update, bool important = true) = 0;
	};

};
