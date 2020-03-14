#pragma once

#include "WasabiGame/Main.hpp"
#include "WasabiGame/Units/AI.hpp"
#include "WasabiGame/Units/Units.hpp"

#include "WasabiGame/Networking/Data.hpp"
#include "RollTheBall/Networking/Protocol.hpp"


namespace RollTheBall {

	class RTBAI : public WasabiGame::AI {
		float m_updateTimer;
		WasabiGame::NetworkUpdate m_unitUpdate;

		struct {
			float angle;
			bool forward;
			bool backward;
			bool left;
			bool right;
			bool jump;
			bool jumpReady;
		} m_movement;

	protected:
		std::weak_ptr<WasabiGame::WasabiBaseGame> m_app;

		void SendNetworkUpdate(WasabiGame::NetworkUpdate& update);

	public:
		RTBAI(std::shared_ptr<WasabiGame::Unit> unit);
		virtual ~RTBAI();

		virtual void Update(float fDeltaTime);

		virtual void OnNetworkUpdate(std::string prop, void* data, size_t size);

		virtual void SetYawAngle(float angle);
		virtual void SetMoveForward(bool isActive);
		virtual void SetMoveBackward(bool isActive);
		virtual void SetMoveLeft(bool isActive);
		virtual void SetMoveRight(bool isActive);
		virtual void SetMoveJump(bool isActive);
		virtual float GetYawAngle() const;
		virtual bool GetMoveForward() const;
		virtual bool GetMoveBackward() const;
		virtual bool GetMoveLeft() const;
		virtual bool GetMoveRight() const;
		virtual bool GetMoveJump() const;
	};

};
