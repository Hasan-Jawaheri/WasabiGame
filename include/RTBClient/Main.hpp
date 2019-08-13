#pragma once

#include "WasabiGame/Main.hpp"

#include "RTBClient/Networking/Networking.hpp"

class RTB : public WasabiRPG {
public:
	RTB();
	~RTB();

	void SwitchToInitialState();

	RTBNet::RTBClientNetworking* RTBNetworking;
};
