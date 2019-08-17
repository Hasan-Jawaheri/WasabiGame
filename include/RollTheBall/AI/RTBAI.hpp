#pragma once

#include "WasabiGame/Units/AI.hpp"
#include "WasabiGame/Units/Units.hpp"

#include "RollTheBall/Networking/Data.hpp"

class RTBAI : public AI {
protected:
	void SentNetworkUpdate(RPGNet::NetworkUpdate& update);

public:
	RTBAI(class Unit* unit);
	virtual ~RTBAI();

	virtual void Update(float fDeltaTime);

	virtual void OnNetworkUpdate(std::string prop, void* data, size_t size);
};


