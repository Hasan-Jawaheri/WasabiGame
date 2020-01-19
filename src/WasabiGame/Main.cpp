#include "WasabiGame/Main.hpp"
#include <Wasabi/Physics/Bullet/WBulletPhysics.hpp>
#include <Wasabi/Renderers/ForwardRenderer/WForwardRenderer.hpp>

#include "WasabiGame/Maps/MapLoader.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/UI/UI.hpp"


WasabiGame::WasabiBaseGame::WasabiBaseGame() : Wasabi(), std::enable_shared_from_this<WasabiBaseGame>() {
	m_settings.debugVulkan = true;
	m_settings.debugPhysics = false;
	m_settings.maxFPS = 0;
	m_settings.mediaFolder = "Media/WasabiGame";
#ifdef _DEBUG
	m_settings.screenWidth = 800;
	m_settings.screenHeight = 600;
	m_settings.fullscreen = false;
#else
	m_settings.screenWidth = GetSystemMetrics(SM_CXSCREEN);
	m_settings.screenHeight = GetSystemMetrics(SM_CYSCREEN);
	m_settings.fullscreen = true;
#endif

	Config = std::make_shared<GameConfig>();
	Scheduler = std::make_shared<GameScheduler>();
	Resources = std::make_shared<ResourceManager>(shared_from_this());
	UI = std::make_shared<UserInterface>(shared_from_this());
	Maps = std::make_shared<MapLoader>(shared_from_this(), Resources);
	Units = std::make_shared<UnitsManager>(shared_from_this(), Resources);
}

WError WasabiGame::WasabiBaseGame::Setup() {
	SetEngineParam<bool>("enableVulkanValidation", m_settings.debugVulkan);

	//
	//Initialize the engine. This must be done to create the window and initialize it
	//
	WError err = StartEngine(m_settings.screenWidth, m_settings.screenHeight);
	maxFPS = m_settings.maxFPS;
	if (m_settings.fullscreen)
		WindowAndInputComponent->SetFullScreenState(m_settings.fullscreen);

	if (err) {
		PhysicsComponent->Start();
		WindowAndInputComponent->SetQuitKeys(false, false);
		CameraManager->GetDefaultCamera()->SetRange(1, 1000);

		err = Resources->Init(m_settings.mediaFolder);
		if (err) {
			err = UI->Init();

			if (err) {
				SwitchToInitialState();
			}
		}
	}

	if (!err) {
		MessageBoxA(nullptr, err.AsString().c_str(), APPNAME, MB_ICONERROR | MB_OK);
		return err;
	}

	return err;
}

bool WasabiGame::WasabiBaseGame::Loop(float fDeltaTime) {
	char text[256];
	sprintf_s(text, 256, "FPS: %.2f//%.2f\nGame Time: %.2f", FPS, maxFPS, Timer.GetElapsedTime());
	TextComponent->RenderText(text, 5, 5, 20, FONT_DEFAULT, WColor(1.0, 0.0, 0.0));

	if (curState) { // we only need this because we destroy state in the windows callback
		fDeltaTime = fmin(fDeltaTime, 1.0f / 15.0f);
		UI->Update(fDeltaTime);
		Maps->Update(fDeltaTime);
		Units->Update(fDeltaTime);
		Resources->Update(fDeltaTime);
	}

	// a null state means exit
	return curState != nullptr; //returning true will allow the application to continue
}

void WasabiGame::WasabiBaseGame::Cleanup() {
	SwitchState(nullptr);
}

WError WasabiGame::WasabiBaseGame::Resize(unsigned int width, unsigned int height) {
	UI->OnResize(width, height);
	return Wasabi::Resize(width, height);
}

WError WasabiGame::WasabiBaseGame::SetupRenderer() {
	return Wasabi::SetupRenderer();
}

WPhysicsComponent* WasabiGame::WasabiBaseGame::CreatePhysicsComponent() {
	WBulletPhysics* physics = new WBulletPhysics(this);
	WError werr = physics->Initialize(m_settings.debugPhysics);
	if (!werr)
		W_SAFE_DELETE(physics);
	return physics;
}
