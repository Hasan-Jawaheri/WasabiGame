#pragma once

#include "RTBClient/Networking/Networking.hpp"


namespace RTBClient {

	class MovementNetworkingManager {
		std::weak_ptr<ClientNetworking> m_clientNetworking;

	public:
		MovementNetworkingManager(std::weak_ptr<ClientNetworking> clientNetworking);

		void Update(float fDeltaTime);

		void SetYawAngle(float angle);
		void SetMoveForward(bool isActive);
		void SetMoveBackward(bool isActive);
		void SetMoveLeft(bool isActive);
		void SetMoveRight(bool isActive);
		void SetMoveJump(bool isActive);
	};

};
