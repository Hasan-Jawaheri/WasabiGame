#pragma once

#include "RollTheBall/AI/RTBAI.hpp"


namespace RollTheBall {

	struct MOVEMENT_PACKET_STRUCT {
		float time;
		char type;
		union PROP {
			PROP() : pos() {}

			WVector3 pos;
			float angle;
			bool state;
		} prop;
	};

	class RemoteControlledAI : public RTBAI {
		std::vector<MOVEMENT_PACKET_STRUCT> m_replayStates;
		WVector3 m_drift; // this is how off we are from we are supposed to be, should slowly decrease over time

	public:
		RemoteControlledAI(std::shared_ptr<class WasabiGame::Unit> unit);
		virtual ~RemoteControlledAI();

		virtual void Update(float fDeltaTime) override;

		virtual void OnNetworkUpdate(std::string prop, void* data, size_t size) override;
	};

};
