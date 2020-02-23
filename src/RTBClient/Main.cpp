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

	Config = std::make_shared<WasabiGame::GameConfig>();
	Scheduler = std::make_shared<WasabiGame::GameScheduler>();
	Networking = std::make_shared<RTBClient::ClientNetworking>(Config, Scheduler);
	Networking->Initialize();
}

RTBClient::ClientApplication::~ClientApplication() {
	Networking->Destroy();
	Scheduler->Stop();

	Networking.reset();
	Scheduler.reset();
	Config.reset();
}

void RTBClient::ClientApplication::SwitchToInitialState() {
	RollTheBall::SetupRTBMaps(Maps);
	RollTheBall::SetupRTBUnits(Units, false);

	PhysicsComponent->SetGravity(0, -40, 0);

#ifndef _DEBUG
	SwitchState(new IntroGameState(this, { "Media/RollTheBall/me3.jpg" }, [this]() { return new MenuGameState(this, [this]() { return new MainGameState(this); }); }));
#else
	SwitchState(new MainGameState(this));
#endif
}
