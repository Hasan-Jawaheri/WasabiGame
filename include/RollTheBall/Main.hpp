#pragma once

#include "WasabiGame/Main.hpp"
#include "RollTheBall/Networking/Data.hpp"

class WasabiRTB : public WasabiRPG {
public:
	WasabiRTB() : WasabiRPG() {}

	virtual void SendNetworkUpdate(RPGNet::NetworkUpdate& update, bool important = true) = 0;
};
