#include "Main.hpp"
#include <Physics/Bullet/WBulletPhysics.h>
#include <Renderers/ForwardRenderer/WForwardRenderer.h>

#include "../WasabiGame/Maps/MapLoader.hpp"
#include "../WasabiGame/Units/UnitsManager.hpp"
#include "../WasabiGame/ResourceManager/ResourceManager.hpp"
#include "../WasabiGame/UI/UI.hpp"

Wasabi* APPHANDLE = nullptr;

WasabiRPG::WasabiRPG() : Wasabi() {
}

WError WasabiRPG::Setup() {
	//
	//Initialize the engine. This must be done to create the window and initialize it
	//
	int scrW = GetSystemMetrics(SM_CXSCREEN);
	int scrH = GetSystemMetrics(SM_CYSCREEN);
#ifdef _DEBUG
	int width = 800;
	int height = 600;
#else
	int width = scrW;
	int height = scrH;
#endif
	bool vsync = false;//enable vsync (prevents screen tearing)
	WError err = StartEngine(width, height);
	maxFPS = 0; // no limit for max FPS
#ifndef _DEBUG
	WindowAndInputComponent->SetFullScreenState(true);
#endif

	if (err) {
		PhysicsComponent->Start();
		WindowAndInputComponent->DisableEscapeKeyQuit();
		CameraManager->GetDefaultCamera()->SetRange(1, 1000);

		err = ResourceManager::Init();
		if (err) {
			err = UserInterface::Init(this);

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
		UserInterface::Update(fDeltaTime);
		MapLoader::Update(fDeltaTime);
		UnitsManager::Update(fDeltaTime);
	}

	// a null state means exit
	return curState != nullptr; //returning true will allow the application to continue
}

void WasabiRPG::Cleanup() {
	UserInterface::Terminate();
	MapLoader::Cleanup();
	ResourceManager::Cleanup();
}

WError WasabiRPG::Resize(unsigned int width, unsigned int height) {
	UserInterface::OnResize(width, height);
	return Wasabi::Resize(width, height);
}

WError WasabiRPG::SetupRenderer() {
	return Wasabi::SetupRenderer();
}

WPhysicsComponent* WasabiRPG::CreatePhysicsComponent() {
	WBulletPhysics* physics = new WBulletPhysics(this);
	WError werr = physics->Initialize();
	if (!werr)
		W_SAFE_DELETE(physics);
	return physics;
}
