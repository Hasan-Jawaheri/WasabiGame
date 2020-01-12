#pragma once

#include "WasabiGame/Units/AI.hpp"
#include "WasabiGame/Units/Units.hpp"

#include "RollTheBall/Networking/Data.hpp"
#include "RollTheBall/Networking/Protocol.hpp"

class RTBAI : public AI {
	float m_updateTimer;
	RPGNet::NetworkUpdate m_unitUpdate;

protected:
	Wasabi* m_app;

	void SendNetworkUpdate(RPGNet::NetworkUpdate& update);

public:
	RTBAI(class Unit* unit);
	virtual ~RTBAI();

	virtual void Update(float fDeltaTime);

	virtual void OnNetworkUpdate(std::string prop, void* data, size_t size);
};


