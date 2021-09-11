#pragma once

#include "WasabiGame/GameStates/BaseState.hpp"
#include "WasabiGame/Utilities/Scheduler.hpp"
#include "RTBServer/Main.hpp"
#include "RTBServer/Simulation/GameStateSyncBackend.hpp"
#include "RTBServer/Game/RTBConnectedPlayer.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"

#include <atomic>


namespace RTBServer {

	class ServerApplication;

	class ServerSimulation {
		std::weak_ptr<ServerApplication> m_server;

		std::unique_ptr<GameStateSyncBackend> m_gameStateSync;

		// this is only used for physics, it is not properly initialized
		std::shared_ptr<WasabiGame::WasabiBaseGame> m_wasabi;

		std::chrono::steady_clock::time_point m_lastUpdateStart;
		double m_targetUpdateDurationMs;
		// this is counting the additional milliseconds that we slept for that we shouldn't have
		double m_millisecondsSleptBeyondTarget;

		void InitializeFPSRegulation();
		double RegulateFPS();

		RollTheBall::MAP_NAME m_map;
		uint32_t m_currentUnitId;
		std::mutex m_unitIdsMutex;

		uint32_t GenerateUnitId();

	public:
		ServerSimulation(std::weak_ptr<ServerApplication> server);
		virtual ~ServerSimulation();

		void Initialize(RollTheBall::MAP_NAME map);
		void Update();
		void Cleanup();

		void AddPlayer(std::shared_ptr<RTBConnectedPlayer> player);
		void RemovePlayer(std::shared_ptr<RTBConnectedPlayer> player);
		bool OnReceivedNetworkUpdate(std::shared_ptr<RTBConnectedPlayer> client, WasabiGame::NetworkUpdate update);
	};

};
