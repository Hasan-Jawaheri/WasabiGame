#include "WasabiGame/GameStates/Menu.hpp"
#include "WasabiGame/Main.hpp"


WasabiGame::MenuGameState::MenuGameState(Wasabi* app, std::function<BaseGameState* ()> nextStateGenerator) : BaseGameState(app) {
	m_nextStateGenerator = nextStateGenerator;
}

WasabiGame::MenuGameState::~MenuGameState() {
}

void WasabiGame::MenuGameState::Load() {
	m_app->SwitchState(m_nextStateGenerator());
}

void WasabiGame::MenuGameState::Update(float fDeltaTime) {
}

void WasabiGame::MenuGameState::Cleanup() {
}
