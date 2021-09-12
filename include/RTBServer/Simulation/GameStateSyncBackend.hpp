#pragma once

#include "RTBServer/Networking/Networking.hpp"
#include "RollTheBall/Networking/GameStateSyncProtocol.hpp"

class WTimer;

namespace RTBServer {

	class GameStateSyncBackend {
		struct PLAYER_INFO {
			RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT latestInput;
			std::shared_ptr<WasabiGame::Unit> unit;
			RollTheBall::UpdateBuilders::GameStateSync::PacketsBuffer<RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT> inputsBuffer;
		};

		std::shared_ptr<ServerNetworking> m_networking;
		WTimer* m_timer;

		std::unordered_map<uint32_t, std::pair<std::shared_ptr<RTBConnectedPlayer>, std::shared_ptr<PLAYER_INFO>>> m_players;
		std::mutex m_playersMutex;
		float m_lastBroadcastTime;
		RollTheBall::UpdateBuilders::GameStateSync::SEQUENCE_NUMBER_TYPE m_motionStatesCurrentSequenceNumber;

	public:
		GameStateSyncBackend(std::shared_ptr<ServerNetworking> networking, WTimer* timer);

		void Update(float fDeltaTime);

		void AddPlayer(std::shared_ptr<RTBServer::RTBConnectedPlayer> player, std::shared_ptr<WasabiGame::Unit> unit);
		std::shared_ptr<WasabiGame::Unit> RemovePlayer(uint32_t clientId);
		bool OnPlayerInputUpdate(std::shared_ptr<RTBServer::RTBConnectedPlayer> player, std::vector<RollTheBall::UpdateBuilders::GameStateSync::INPUT_STRUCT>& inputStructs);
	};

}
