#include "WasabiGame/Main.hpp"
#include <Wasabi/Physics/Bullet/WBulletPhysics.hpp>
#include <Wasabi/Renderers/ForwardRenderer/WForwardRenderer.hpp>

#include "WasabiGame/Maps/MapLoader.hpp"
#include "WasabiGame/Units/UnitsManager.hpp"
#include "WasabiGame/ResourceManager/ResourceManager.hpp"
#include "WasabiGame/UI/UI.hpp"

WasabiRPG::WasabiRPG() : Wasabi() {
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

	Resources = new ResourceManager(this);
	UI = new UserInterface(this);
	Maps = new MapLoader(this, Resources);
	Units = new UnitsManager(this, Resources);
}

WError WasabiRPG::Setup() {
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
			err = UI->Init(this);

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

bool WasabiRPG::Loop(float fDeltaTime) {
	fCurGameTime = Timer.GetElapsedTime();

	char text[256];
	sprintf_s(text, 256, "FPS: %.2f//%.2f\nGame Time: %.2f", FPS, maxFPS, fCurGameTime);
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

void WasabiRPG::Cleanup() {
	SwitchState(nullptr);

	UI->Terminate();
	Maps->Cleanup();
	Units->Cleanup();
	Resources->Cleanup();

	W_SAFE_DELETE(UI);
	W_SAFE_DELETE(Maps);
	W_SAFE_DELETE(Units);
	W_SAFE_DELETE(Resources);
}

WError WasabiRPG::Resize(unsigned int width, unsigned int height) {
	UI->OnResize(width, height);
	return Wasabi::Resize(width, height);
}

WError WasabiRPG::SetupRenderer() {
	return Wasabi::SetupRenderer();
}

WPhysicsComponent* WasabiRPG::CreatePhysicsComponent() {
	WBulletPhysics* physics = new WBulletPhysics(this);
	WError werr = physics->Initialize(m_settings.debugPhysics);
	if (!werr)
		W_SAFE_DELETE(physics);
	return physics;
}
