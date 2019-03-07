#include "Menu.hpp"
#include "Game.hpp"

Menu::Menu(Wasabi* app) : WGameState(app) {
}

Menu::~Menu() {
}

void Menu::Load() {
	m_app->SwitchState(new Game(m_app));
}

void Menu::Update(float fDeltaTime) {
}

void Menu::Cleanup() {
}
