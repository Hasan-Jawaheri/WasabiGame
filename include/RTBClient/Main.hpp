#pragma once

#include <WinSock2.h>
#include "WasabiGame/Main.hpp"

#include "RTBClient/Networking/Networking.hpp"

class RTB : public WasabiRPG {
public:
	RTB(bool generateAssets = true, bool enableVulkanDebugging = true, bool enablePhysicsDebugging = true);
	~RTB();

	void SwitchToInitialState();

	RTBNet::RTBClientNetworking* RTBNetworking;
};
