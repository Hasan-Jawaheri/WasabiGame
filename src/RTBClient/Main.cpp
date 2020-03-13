#include "RTBClient/Main.hpp"

#include "RollTheBall/Main.hpp"
#include "WasabiGame/GameStates/Intro.hpp"
#include "WasabiGame/GameStates/Menu.hpp"
#include "RTBClient/GameStates/MainGameState.hpp"
#include "RollTheBall/Maps/RTBMaps.hpp"
#include "RollTheBall/Units/RTBUnits.hpp"

#include "RollTheBall/AssetsGenerator/AssetsGenerator.hpp"

RTBClient::ClientApplication::ClientApplication(bool generateAssets, bool enableVulkanDebugging, bool enablePhysicsDebugging) : WasabiGame::WasabiBaseGame() {
	SetEngineParam("appName", "RTBClient");

#ifdef _DEBUG
	m_settings.debugVulkan = enableVulkanDebugging;
	m_settings.debugPhysics = enablePhysicsDebugging;
	m_settings.fullscreen = false;
#else
	m_settings.debugVulkan = false;
	m_settings.debugPhysics = false;
	m_settings.fullscreen = false;
#endif

	m_settings.mediaFolder = "Media/RollTheBall";

	if (generateAssets) {
		if (!RollTheBall::AssetGenerator(m_settings.mediaFolder).Generate())
			return;
	}
}

RTBClient::ClientApplication::~ClientApplication() {
}

void RTBClient::ClientApplication::SwitchToInitialState() {
	Networking = std::make_shared<RTBClient::ClientNetworking>(shared_from_this(), Config, Scheduler);
	Networking->Initialize();
	
	RollTheBall::SetupRTBMaps(Maps);
	RollTheBall::SetupRTBUnits(Units, false);

	PhysicsComponent->SetGravity(0, -40, 0);

#ifndef _DEBUG
	SwitchState(
		new WasabiGame::IntroGameState(
			this,
			{ "Media/RollTheBall/me3.jpg" },
			[this]() {
				return new WasabiGame::MenuGameState(this, [this]() {
					return new MainGameState(this);
				});
			}
		)
	);
#else
	SwitchState(new MainGameState(this));
#endif
}
