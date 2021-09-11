#pragma once

#include "RTBServer/Networking/Networking.hpp"

class WTimer;

namespace RTBServer {

	class GameStateSyncBackend {
		std::shared_ptr<ServerNetworking> m_networking;
		WTimer* m_timer;

		std::unordered_map<uint32_t, std::pair<std::shared_ptr<RTBConnectedPlayer>, std::shared_ptr<WasabiGame::Unit>>> m_players;
		std::mutex m_playersMutex;
		float m_lastBroadcastTime;

	public:
		GameStateSyncBackend(std::shared_ptr<ServerNetworking> networking, WTimer* timer);

		void Update(float fDeltaTime);

		void AddPlayer(std::shared_ptr<RTBServer::RTBConnectedPlayer> player, std::shared_ptr<WasabiGame::Unit> unit);
		std::shared_ptr<WasabiGame::Unit> RemovePlayer(uint32_t clientId);
	};

}
