#pragma once

#include "RTBClient/Networking/Networking.hpp"
#include "WasabiGame/Units/Units.hpp"
#include "Wasabi/Core/WTimer.hpp"


namespace RTBClient {

	class MovementNetworkingManager {
		std::weak_ptr<ClientNetworking> m_clientNetworking;

		::WTimer* m_clientTimer;
		float m_periodicUpdateTimer;
		WasabiGame::NetworkUpdate m_update;
		WVector3 m_playerPosition;

	public:
		MovementNetworkingManager(std::weak_ptr<ClientNetworking> clientNetworking, ::WTimer* clientTimer);

		void Update(float fDeltaTime);

		void UpdatePlayer(std::shared_ptr<WasabiGame::Unit> player);
		void SetYawAngle(float angle);
		void SetMoveForward(bool isActive);
		void SetMoveBackward(bool isActive);
		void SetMoveLeft(bool isActive);
		void SetMoveRight(bool isActive);
		void SetMoveJump(bool isActive);
	};

};
