#include "Main.hpp"
#ifndef _DEBUG
#include "GameStates/Intro.hpp"
#else
#include "GameStates/Game.hpp"
#endif
//#include "Maps/MapLoader.hpp" @TODO: CHANGE HERE
//#include "Entities/Player.hpp" @TODO: CHANGE HERE
//#include "Spells/Spell.hpp" @TODO: CHANGE HERE
#include "UI/UI.hpp"

Application* APPHANDLE;

Application::Application() : Wasabi() {
}

WError Application::Setup() {
	//
	//Initialize the engine. This must be done to create the window and initialize it
	//
	int scrW = GetSystemMetrics(SM_CXSCREEN);
	int scrH = GetSystemMetrics(SM_CYSCREEN);
#ifdef _DEBUG
	int width = 640;	//640 pixels wide
	int height = 480;	//480 pixels high
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
	WindowComponent->SetFullScreenState(true);
#endif

	InputComponent->DisableEscapeKeyQuit();
	CameraManager->GetDefaultCamera()->SetRange(1, 10000);

	//err = PhysicsComponent->Initialize();
	//if (!err) {
	//	MessageBoxA(nullptr, err.AsString().c_str(), APPNAME, MB_ICONERROR | MB_OK);
	//	return err;
	//}
	// PhysicsComponent->Init(4.0f, 15000, true);
	// PhysicsComponent->SetGravity(0.0f, -12.0f, 0.0f);
	//PhysicsComponent->Start();

	//ResourceManager::Init(this); @TODO: CHANGE HERE
	//MapLoader::Init(); @TODO: CHANGE HERE
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
	}

	//PhysicsComponent->Step(fDeltaTime); @TODO: CHANGE HERE

	//a null state means exit
	return curState != nullptr; //returning true will allow the application to continue
}

void Application::Cleanup() {
	UserInterface::Terminate();
	//MapLoader::Cleanup(); @TODO: CHANGE HERE
	//ResourceManager::Cleanup(); @TODO: CHANGE HERE
}

WError Application::Resize(unsigned int width, unsigned int height) {
	UserInterface::OnResize(width, height);
	return Wasabi::Resize(width, height);
}

Wasabi* WInitialize() {
	return new Application();
}