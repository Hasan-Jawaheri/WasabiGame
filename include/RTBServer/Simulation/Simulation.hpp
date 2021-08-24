#pragma once

#include "WasabiGame/GameStates/BaseState.hpp"
#include "WasabiGame/Utilities/Scheduler.hpp"
#include "RTBServer/Main.hpp"
#include "RTBServer/Game/RTBConnectedPlayer.hpp"

#include <atomic>


namespace RTBServer {

	class ServerApplication;

	class ServerSimulation {
		std::weak_ptr<ServerApplication> m_server;

		// this is only used for physics, it is not properly initialized
		class ::Wasabi* m_wasabi;

		std::chrono::steady_clock::time_point m_lastUpdateStart;
		double m_targetUpdateDurationMs;
		// this is counting the additional milliseconds that we slept for that we shouldn't have
		double m_millisecondsSleptBeyondTarget;

		void InitializeFPSRegulation();
		double RegulateFPS();

		struct {
			float fYaw, fPitch, fDist;
			WVector3 vPos;
			bool bMouseHidden;
			int lastX, lastY;
		} m_cam;

		uint32_t m_currentUnitId;
		std::mutex m_unitIdsMutex;

		std::unordered_map<uint32_t, std::pair<std::shared_ptr<RTBConnectedPlayer>, std::shared_ptr<WasabiGame::Unit>>> m_players;
		std::mutex m_playersMutex;
		float m_lastBroadcastTime;

		uint32_t GenerateUnitId();
		void ApplyMousePivot();

	public:
		ServerSimulation(std::weak_ptr<ServerApplication> server);
		virtual ~ServerSimulation();

		void Initialize();
		void Update();
		void Cleanup();

		void AddPlayer(std::shared_ptr<RTBConnectedPlayer> player);
		void RemovePlayer(std::shared_ptr<RTBConnectedPlayer> player);
	};

};
