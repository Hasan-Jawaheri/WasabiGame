#pragma once

#include "WasabiGame/Main.hpp"

#include "RTBClient/Networking/Networking.hpp"

class RTB : public WasabiRPG {
public:
	RTB(bool generateAssets = true, bool enableVulkanDebugging = true);
	~RTB();

	void SwitchToInitialState();

	RTBNet::RTBClientNetworking* RTBNetworking;
};
