#pragma once

#include "WasabiGame/GameStates/BaseState.hpp"
#include "WasabiGame/Utilities/Scheduler.hpp"
#include "RTBServer/Main.hpp"
#include "RTBServer/Game/RTBConnectedPlayer.hpp"

#include <atomic>


namespace RTBServer {

	class ServerApplication;

	class ServerSimulationGameState : public WasabiGame::BaseGameState {
		ServerApplication* m_server;

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
		ServerSimulationGameState(Wasabi* app);
		virtual ~ServerSimulationGameState();

		virtual void Load();
		virtual void Update(float fDeltaTime);
		virtual void Cleanup();

		void AddPlayer(std::shared_ptr<RTBConnectedPlayer> player);
		void RemovePlayer(std::shared_ptr<RTBConnectedPlayer> player);
	};

};
