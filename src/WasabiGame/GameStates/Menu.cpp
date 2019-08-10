#include "WasabiGame/GameStates/Menu.hpp"

Menu::Menu(Wasabi* app, std::function<WGameState* ()> nextStateGenerator) : WGameState(app) {
	m_nextStateGenerator = nextStateGenerator;
}

Menu::~Menu() {
}

void Menu::Load() {
	m_app->SwitchState(m_nextStateGenerator());
}

void Menu::Update(float fDeltaTime) {
}

void Menu::Cleanup() {
}
