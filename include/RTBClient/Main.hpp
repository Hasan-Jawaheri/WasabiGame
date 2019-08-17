#pragma once

#include <WinSock2.h>
#include "WasabiGame/Main.hpp"

#include "RTBClient/Networking/Networking.hpp"

class RTBClient : public WasabiRPG {
public:
	RTBClient(bool generateAssets = true, bool enableVulkanDebugging = true, bool enablePhysicsDebugging = true);
	~RTBClient();

	void SwitchToInitialState();

	RTBNet::RTBClientNetworking* Networking;
};
