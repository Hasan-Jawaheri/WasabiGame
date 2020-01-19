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

	protected:
		std::weak_ptr<WasabiGame::WasabiBaseGame> m_app;

		void SendNetworkUpdate(WasabiGame::NetworkUpdate& update);

	public:
		RTBAI(std::shared_ptr<WasabiGame::Unit> unit);
		virtual ~RTBAI();

		virtual void Update(float fDeltaTime);

		virtual void OnNetworkUpdate(std::string prop, void* data, size_t size);
	};

};
