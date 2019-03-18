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

/*int Application::WindowCallback(HWND wnd, UINT msg, WPARAM w, LPARAM l, bool bBeforeProcessing) {
	if (!curState)
		return 0;

	// Handle input & window changes
	if (!bBeforeProcessing) {
		int mx = LOWORD(l);
		int my = HIWORD(l);

		if (msg == WM_SIZE) {
			UserInterface::OnResize(WindowComponent->GetWindowWidth(), WindowComponent->GetWindowHeight());
		}

		if (msg == WM_MOUSEMOVE) {
			UIElement* targetElement = UserInterface::GetElementAt(mx, my);
			if (targetElement)
				targetElement->OnMouseMove(mx, my);
		}

		if (msg == WM_LBUTTONDOWN) {
			UIElement* targetElement = UserInterface::GetElementAt(mx, my);
			if (targetElement) {
				targetElement->OnMouseButton(mx, my, true);
				if (targetElement->GetSizeX() != 0 && targetElement->GetSizeY() != 0) {
					bool bGiveFocus = true;
					if (UserInterface::GetFocus())
						if (!UserInterface::GetFocus()->OnLoseFocus()) {
							bool bIsChild = false;
							for (UINT i = 0; i < UserInterface::GetFocus()->GetNumChildren(); i++)
								if (UserInterface::GetFocus()->GetChild(i) == targetElement)
									bIsChild = true;
							if (!bIsChild)
								bGiveFocus = false;
						}

					if (targetElement->OnFocus() && bGiveFocus)
						UserInterface::SetFocus(targetElement);
				}
			}
		}

		if (msg == WM_LBUTTONUP) {
			UIElement* targetElement = UserInterface::GetElementAt(mx, my);
			if (targetElement) {
				bool bProcceed = true;
				if (UserInterface::GetFocus())
					if (!UserInterface::GetFocus()->OnLoseFocus()) {
						bool bIsChild = false;
						for (UINT i = 0; i < UserInterface::GetFocus()->GetNumChildren(); i++)
							if (UserInterface::GetFocus()->GetChild(i) == targetElement)
								bIsChild = true;
						if (!bIsChild)
							bProcceed = false;
					}

				if (bProcceed)
					targetElement->OnMouseButton(mx, my, false);
			}
		}

		if (msg == WM_RBUTTONDOWN) {
			UIElement* targetElement = UserInterface::GetElementAt(mx, my);
			if (targetElement)
				targetElement->OnMouseButton2(mx, my, true);
		}

		if (msg == WM_RBUTTONUP) {
			UIElement* targetElement = UserInterface::GetElementAt(mx, my);
			if (targetElement)
				targetElement->OnMouseButton2(mx, my, false);
		}

		if (msg == WM_KEYDOWN && UserInterface::GetFocus()) {
			if (UserInterface::GetFocus()->OnInput())
				UserInterface::GetFocus()->OnKeydown(w);
			else //if it does not accept input, see if it's parent does
				if (UserInterface::GetFocus()->GetParent())
					if (UserInterface::GetFocus()->GetParent()->OnInput())
						UserInterface::GetFocus()->GetParent()->OnKeydown(w);
		}

		if (msg == WM_KEYUP && UserInterface::GetFocus()) {
			if (UserInterface::GetFocus()->OnInput())
				UserInterface::GetFocus()->OnKeyup(w);
			else //if it does not accept input, see if it's parent does
				if (UserInterface::GetFocus()->GetParent())
					if (UserInterface::GetFocus()->GetParent()->OnInput())
						UserInterface::GetFocus()->GetParent()->OnKeyup(w);
		}

		if (msg == WM_CHAR && UserInterface::GetFocus()) {
			int repeat = LOWORD(l);
			for (UINT i = 0; i < repeat; i++) {
				if (w == VK_RETURN) {
					UIElement* parent = UserInterface::GetFocus()->GetParent();
					if (UserInterface::GetFocus()->OnEnter()) {
						if (parent)
							parent->OnEnter();
						continue;
					}
				} else if (w == VK_ESCAPE) {
					if (UserInterface::GetFocus()->OnEscape()) {
						if (UserInterface::GetFocus()->GetParent())
							UserInterface::GetFocus()->GetParent()->OnEscape();
						continue;
					}
				} else if (w == VK_TAB) {
					if (UserInterface::GetFocus()->OnTab()) {
						if (UserInterface::GetFocus()->GetParent() && UserInterface::GetFocus()->OnLoseFocus()) {
							UINT numChildren = UserInterface::GetFocus()->GetParent()->GetNumChildren();
							UINT focusIndex = -1;
							for (UINT i = 0; i <= numChildren; i++) {
								if (i == numChildren)
									i = 0;

								if (focusIndex != -1) {
									if (UserInterface::GetFocus()->GetParent()->GetChild(i)->OnTab()) {
										if (UserInterface::GetFocus()->GetParent()->GetChild(i)->OnFocus())
											UserInterface::SetFocus(UserInterface::GetFocus()->GetParent()->GetChild(i));
										break;
									}
								}

								if (UserInterface::GetFocus()->GetParent()->GetChild(i) == UserInterface::GetFocus())
									focusIndex = i;
							}
						}
						continue;
					}
				} else if (w == VK_BACK) {
					if (strlen(UserInterface::GetFocus()->GetInputBuffer()) && UserInterface::GetFocus()->OnInput())
						UserInterface::GetFocus()->GetInputBuffer()[strlen(UserInterface::GetFocus()->GetInputBuffer()) - 1] = '\0';
					continue;
				}

				if (UserInterface::GetFocus()->OnInput()) {
					if (strlen(UserInterface::GetFocus()->GetInputBuffer()) <
						UserInterface::GetFocus()->GetInputBufferSize() - 1)
						sprintf_s(UserInterface::GetFocus()->GetInputBuffer(), UserInterface::GetFocus()->GetInputBufferSize(),
							"%s%c", UserInterface::GetFocus()->GetInputBuffer(), w);
				} else //if it does not accept input, see if it's parent does
				{
					if (UserInterface::GetFocus()->GetParent())
						if (UserInterface::GetFocus()->GetParent()->OnInput()) {
							if (strlen(UserInterface::GetFocus()->GetParent()->GetInputBuffer()) <
								UserInterface::GetFocus()->GetParent()->GetInputBufferSize() - 1)
								sprintf_s(UserInterface::GetFocus()->GetParent()->GetInputBuffer(),
									UserInterface::GetFocus()->GetParent()->GetInputBufferSize(),
									"%s%c", UserInterface::GetFocus()->GetParent()->GetInputBuffer(), w);
						}
				}
			}
		}
	}

	return 0;
}*/

Wasabi* WInitialize() {
	return new Application();
}