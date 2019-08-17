#pragma once

#include "RollTheBall/Main.hpp"

#include "RTBClient/Networking/Networking.hpp"

class RTBClient : public WasabiRTB {
public:
	RTBClient(bool generateAssets = true, bool enableVulkanDebugging = true, bool enablePhysicsDebugging = true);
	~RTBClient();

	void SwitchToInitialState();

	virtual void SendNetworkUpdate(RPGNet::NetworkUpdate& update, bool important = true);

	RTBNet::RTBClientNetworking* Networking;
};
