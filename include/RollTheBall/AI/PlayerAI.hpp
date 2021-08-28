#pragma once

#include "RollTheBall/AI/RTBAI.hpp"
#include "RollTheBall/Units/Player.hpp"
#include "Wasabi/Core/WTimer.hpp"


namespace RollTheBall {
	
	class PlayerAI : public RTBAI {
		WCamera* m_camera;
		WVector3 m_cameraPivot;
		float m_cameraPitch, m_cameraDistance;

		::WTimer* m_clientTimer;
		float m_periodicUpdateTimer;
		WasabiGame::NetworkUpdate m_update;

		void SendMovementUpdate(void* update, size_t size);
		void SendMovementUpdate(char type, float angle);
		void SendMovementUpdate(char type, WVector3 position);
		void SendMovementUpdate(char type, bool state);

	public:
		PlayerAI(std::shared_ptr<class WasabiGame::Unit> unit);
		virtual ~PlayerAI();

		virtual void Update(float fDeltaTime);

		void SetCameraPitch(float pitch);
		void SetCameraDistance(float distance);
		float GetCameraPitch() const;
		float GetCameraDistance() const;

		virtual void OnNetworkUpdate(std::string prop, void* data, size_t size) override;
		virtual void SetYawAngle(float angle) override;
		virtual void SetMoveForward(bool isActive) override;
		virtual void SetMoveBackward(bool isActive) override;
		virtual void SetMoveLeft(bool isActive) override;
		virtual void SetMoveRight(bool isActive) override;
		virtual void SetMoveJump(bool isActive) override;
	};

};
