#include "Main.hpp"
#include <Physics/Bullet/WBulletPhysics.h>
#include <Renderers/ForwardRenderer/WForwardRenderer.h>
#ifndef _DEBUG
#include "GameStates/Intro.hpp"
#else
#include "GameStates/Game.hpp"
#endif
#include "Maps/MapLoader.hpp"
#include "ResourceManager/ResourceManager.hpp"
#include "UI/UI.hpp"

Application* APPHANDLE = nullptr;

Application::Application() : Wasabi() {
}

WError Application::Setup() {
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
	if (!err) {
		MessageBoxA(nullptr, err.AsString().c_str(), APPNAME, MB_ICONERROR | MB_OK);
		return err;
	}
	maxFPS = 0; // no limit for max FPS
#ifndef _DEBUG
	WindowAndInputComponent->SetFullScreenState(true);
#endif

	PhysicsComponent->Start();
	WindowAndInputComponent->DisableEscapeKeyQuit();
	CameraManager->GetDefaultCamera()->SetRange(1, 1000);

	ResourceManager::Init();
	err = UserInterface::Init(this);

#ifndef _DEBUG
	SwitchState(new Intro(this));
#else
	SwitchState(new Game(this));
#endif

	return err;
}

bool Application::Loop(float fDeltaTime) {
	fCurGameTime = Timer.GetElapsedTime();

	char text[256];
	sprintf_s(text, 256, "FPS: %.2f//%.2f\nGame Time: %.2f", FPS, maxFPS, fCurGameTime);
	TextComponent->RenderText(text, 5, 5, 20, FONT_DEFAULT, WColor(1.0, 0.0, 0.0));

	if (curState) { // we only need this because we destroy state in the windows callback
		UserInterface::Update(fDeltaTime);
		MapLoader::Update(fDeltaTime);
	}

	// a null state means exit
	return curState != nullptr; //returning true will allow the application to continue
}

void Application::Cleanup() {
	UserInterface::Terminate();
	MapLoader::Cleanup();
	ResourceManager::Cleanup();
}

WError Application::Resize(unsigned int width, unsigned int height) {
	UserInterface::OnResize(width, height);
	return Wasabi::Resize(width, height);
}

WRenderer* Application::CreateRenderer() {
	return new WForwardRenderer(this);
}

WPhysicsComponent* Application::CreatePhysicsComponent() {
	WBulletPhysics* physics = new WBulletPhysics(this);
	WError werr = physics->Initialize();
	if (!werr)
		W_SAFE_DELETE(physics);
	return physics;
}

Wasabi* WInitialize() {
	APPHANDLE = new Application();
	return APPHANDLE;
}
